module Vulkan: ImageImp;
import: Image;
import: Enum;
import: Device;
import: Buffer;
import: Commend;
import: Sync;
import: Pipeline;
import: Memory;
import std.core;
import Define;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
using namespace vk;

size_t formatUnitSize(VkFormat format) {
	switch (format) {
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_R32_SFLOAT: return sizeof(float);
	case VK_FORMAT_R32G32_SFLOAT: return sizeof(vec2);
	case VK_FORMAT_R32G32B32_SFLOAT: return sizeof(vec3);
	case VK_FORMAT_R32G32B32A32_SFLOAT: return sizeof(vec4);
	};
	throw "VulkanInvalidFormat";
}

//----------------------------------------------
//------------------Image-----------------------
//----------------------------------------------
Image::Image(LogicalDevice& d, uvec3 texSize, uint texDimension,
  VkFormat texFormat, Flags<TextureUseType> texUsage,
  Flags<MemoryFeature> texMemFeature, Flags<TextureFeature> texFeature,
  uint mipLevels, uint layers, uint subsamples):
  d(d) {
	if (texMemFeature.has(MemoryFeature::Mappable)) {
		if (texMemFeature.has(MemoryFeature::Coherent))
			minAlignment =
			  d.pd.properties10.properties.limits.nonCoherentAtomSize;
		else
			minAlignment =
			  d.pd.properties10.properties.limits.minMemoryMapAlignment;
	} else {
		minAlignment = 1;
	}
	if (texMemFeature.has(MemoryFeature::IndirectWritable)) {
		texUsage.set(TextureUseType::TransferDst);
	}
	if (texMemFeature.has(MemoryFeature::IndirectReadable)) {
		texUsage.set(TextureUseType::TransferSrc);
	}
	// HOTFIX: This is hotfix for min requirement for alignment
	minAlignment = d.pd.properties10.properties.limits.nonCoherentAtomSize;

	size = texSize;
	feature = texFeature;
	memFeature = texMemFeature;
	usage = texUsage;
	dimension = texDimension;
	activeUsage = texMemFeature.has(MemoryFeature::Mappable)
				  ? TextureActiveUseType::HostWritable
				  : TextureActiveUseType::Undefined;
	format = texFormat;
	this->mipLevels = mipLevels;
	this->layers = layers;

	// Cal texMemorySize
	texMemorySize = formatUnitSize(texFormat) * size.x * size.y * size.z;
	if (texMemorySize == 0) throw "VulkanInvalidTextureSize";

	VkImageCreateInfo iInfo{};
	iInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	iInfo.flags = (VkImageCreateFlags)texFeature;
	iInfo.imageType = (VkImageType)(texDimension - 1);	// Hmm...
	iInfo.format = texFormat;
	iInfo.extent = VkExtent3D{texSize.x, texSize.y, texSize.z};
	iInfo.mipLevels = mipLevels;
	iInfo.arrayLayers = layers;
	iInfo.samples = (VkSampleCountFlagBits)subsamples;
	iInfo.tiling = texMemFeature.has(MemoryFeature::Mappable)
				   ? VK_IMAGE_TILING_LINEAR
				   : VK_IMAGE_TILING_OPTIMAL;
	iInfo.usage = (VkImageUsageFlags)texUsage;
	iInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// iInfo.queueFamilyIndexCount;
	// iInfo.pQueueFamilyIndices;
	iInfo.initialLayout = (VkImageLayout)activeUsage;

	if (vkCreateImage(d.d, &iInfo, nullptr, &img) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	// Alloc Memory
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(d.d, img, &memRequirements);
	mSize = memRequirements.size;
	auto pair = d.allocMemory(
	  memRequirements.memoryTypeBits, memFeature, mSize, minAlignment);
	memoryIndex = pair.first;
	ptr = pair.second;
	if (vkBindImageMemory(d.d, img, ptr.dm.dm, ptr.offset) != VK_SUCCESS)
		throw "VulkanImageBindMemoryFailure";
}
Image::Image(Image&& other) noexcept: d(other.d) {
	texMemorySize = other.texMemorySize;
	mSize = other.mSize;
	size = other.size;
	minAlignment = other.minAlignment;
	img = other.img;
	memoryIndex = other.memoryIndex;
	ptr = other.ptr;
	mappedPtr = other.mappedPtr;
	memFeature = other.memFeature;
	usage = other.usage;
	activeUsage = other.activeUsage;
	dimension = other.dimension;
	mipLevels = other.mipLevels;
	layers = other.layers;
	format = other.format;
	feature = other.feature;
	other.img = nullptr;
	other.memoryIndex = uint(-1);
	other.mappedPtr = nullptr;
}
Image::~Image() {
	if (img != nullptr) vkDestroyImage(d.d, img, nullptr);
	if (memoryIndex != uint(-1)) d.freeMemory(memoryIndex, ptr);
}

void* Image::map() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr != nullptr) throw "VulkanImageAlreadyMapped";
		if (activeUsage != TextureActiveUseType::General
			&& activeUsage != TextureActiveUseType::HostWritable)
			throw "VulkanImageIncorrectActiveUsage";
	}
	vkMapMemory(d.d, ptr.dm.dm, ptr.offset, mSize, 0, &mappedPtr);
	return mappedPtr;
}
void* Image::map(size_t nSize, size_t offset) {
	if constexpr (DO_SAFETY_CHECK) {
		if (nSize + offset > texMemorySize) throw "VulkanImageOutOfRange";
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr != nullptr) throw "VulkanImageAlreadyMapped";
		if (nSize % minAlignment != 0) throw "VulkanImageNotOnMinAlignment";
		if (offset % minAlignment != 0) throw "VulkanImageNotOnMinAlignment";
		if (activeUsage != TextureActiveUseType::General
			&& activeUsage != TextureActiveUseType::HostWritable)
			throw "VulkanImageIncorrectActiveUsage";
	}
	vkMapMemory(d.d, ptr.dm.dm, ptr.offset + offset, nSize, 0, &mappedPtr);
	return mappedPtr;
}
void Image::flush() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr == nullptr) throw "VulkanImageNotMapped";
		if (memFeature.has(MemoryFeature::Coherent))
			logger("Warning: Vulkan called flush() on coherent image.\n");
	}
	VkMappedMemoryRange r{};
	r.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	r.memory = ptr.dm.dm;
	r.offset = ptr.offset;
	r.size = mSize;
	vkFlushMappedMemoryRanges(d.d, 1, &r);
}
void Image::flush(size_t nSize, size_t offset) {
	if constexpr (DO_SAFETY_CHECK) {
		if (nSize + offset > texMemorySize) throw "VulkanImageOutOfRange";
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr == nullptr) throw "VulkanImageNotMapped";
		if (nSize % minAlignment != 0) throw "VulkanImageNotOnMinAlignment";
		if (offset % minAlignment != 0) throw "VulkanImageNotOnMinAlignment";
	}
	VkMappedMemoryRange r{};
	r.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	r.memory = ptr.dm.dm;
	r.size = nSize;
	r.offset = ptr.offset + offset;
	vkFlushMappedMemoryRanges(d.d, 1, &r);
}
void Image::unmap() {
	if constexpr (DO_SAFETY_CHECK) {
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr == nullptr) throw "VulkanImageNotMapped";
		if (activeUsage != TextureActiveUseType::General
			&& activeUsage != TextureActiveUseType::HostWritable)
			throw "VulkanImageIncorrectActiveUsage";
		mappedPtr = nullptr;
	}
	// FIXME: Can't unmap part of memory...
	vkUnmapMemory(d.d, ptr.dm.dm);
}

void Image::blockingIndirectWrite(const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!memFeature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
	}
	auto sg = Buffer(d, texMemorySize,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto cb = d.getSingleUseCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);

	cb.cmdCopy(sg, *this, TextureAspect::Color, size, size);

	cb.endRecording();
	cb.submit().wait();
}
void Image::blockingIndirectWrite(
  size_t nSize, size_t offset, const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!memFeature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
		if (nSize + offset > texMemorySize) throw "VulkanBufferOutOfRange";
	}
	auto sg = Buffer(d, nSize,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto cb = d.getSingleUseCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);

	cb.cmdCopy(sg, *this, TextureAspect::Color, size, size);

	cb.endRecording();
	cb.submit().wait();
}
void Image::directWrite(const void* data) {
	memcpy(map(), data, texMemorySize);
	flush();  // FIXME: ????????? Is this needed?
	unmap();
}
void Image::directWrite(size_t nSize, size_t offset, const void* data) {
	memcpy(map(nSize, offset), data, nSize);
	flush();  // FIXME: ????????? Is this needed?
	unmap();
}

void Image::blockingTransformActiveUsage(TextureActiveUseType targetUsage) {
	if constexpr (DO_SAFETY_CHECK) {
		//??? What to check?
	}
	auto cb = d.getSingleUseCommendBuffer();
	cb.startRecording(CommendBufferUsage::Streaming);

	cb.cmdChangeState(*this, targetUsage, PipelineStage::TopOfPipe,
	  MemoryAccess::None, PipelineStage::BottomOfPipe, MemoryAccess::None);

	cb.endRecording();
	cb.submit().wait();
}

//----------------------------------------------
//------------------ImageView-------------------
//----------------------------------------------
ImageView::ImageView(LogicalDevice& d, const Image& img): d(d) {
	VkImageViewCreateInfo cInfo{};
	cInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	cInfo.image = img.img;
	// FIXME: Hack for viewTpye value.
	cInfo.viewType = (VkImageViewType)(img.dimension - 1);
	cInfo.format = img.format;
	cInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cInfo.subresourceRange.baseMipLevel = 0;
	cInfo.subresourceRange.levelCount = img.mipLevels;
	cInfo.subresourceRange.baseArrayLayer = 0;
	cInfo.subresourceRange.layerCount = img.layers;
	if (vkCreateImageView(d.d, &cInfo, nullptr, &imgView) != VK_SUCCESS) {
		logger("Vulkan failed to make Image View form VkImage!\n");
		throw "VulkanCreateImageViewFailure";
	}
}

ImageView::ImageView(LogicalDevice& d, VkImageViewCreateInfo createInfo): d(d) {
	if (vkCreateImageView(d.d, &createInfo, nullptr, &imgView) != VK_SUCCESS) {
		logger("Vulkan failed to make Image View form VkImage!\n");
		throw "VulkanCreateImageViewFailure";
	}
}
ImageView::ImageView(ImageView&& other) noexcept: d(other.d) {
	imgView = other.imgView;
	other.imgView = nullptr;
}
ImageView::~ImageView() {
	if (imgView != nullptr) vkDestroyImageView(d.d, imgView, nullptr);
}

//----------------------------------------------
//------------------ImageSampler----------------
//----------------------------------------------

ImageSampler::ImageSampler(LogicalDevice& d, SamplerFilter minFilter,
  SamplerFilter magFilter, SamplerClampMode clampModeX,
  SamplerClampMode clampModeY, SamplerClampMode clampModeZ,
  SamplerBorderColor borderColor, bool normalized, SamplerMipmapMode mipmapMode,
  float lodBias, float minLod, float maxLod):
  d(d) {
	VkSamplerCreateInfo scInfo{};
	scInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	// scInfo.flags; Only useful for VK_EXT_fragment_density_map
	scInfo.minFilter = (VkFilter)minFilter;
	scInfo.magFilter = (VkFilter)magFilter;
	scInfo.mipmapMode = (VkSamplerMipmapMode)mipmapMode;
	scInfo.addressModeU = (VkSamplerAddressMode)clampModeX;
	scInfo.addressModeV = (VkSamplerAddressMode)clampModeY;
	scInfo.addressModeW = (VkSamplerAddressMode)clampModeZ;
	scInfo.mipLodBias = lodBias;
	// TODO: Add settings for anistropic filtering
	scInfo.anisotropyEnable = VK_TRUE;
	scInfo.maxAnisotropy =
	  d.pd.properties10.properties.limits.maxSamplerAnisotropy;
	// NOTE: Change here to add support for shader map multisample stuff
	scInfo.compareEnable = VK_FALSE;
	scInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	scInfo.minLod = minLod;
	scInfo.maxLod = maxLod;
	scInfo.borderColor = (VkBorderColor)borderColor;
	scInfo.unnormalizedCoordinates = normalized ? VK_FALSE : VK_TRUE;

	if (vkCreateSampler(d.d, &scInfo, nullptr, &smp) != VK_SUCCESS) {
		throw "VulkanImageSamplerCreateFailure";
	}
}

ImageSampler::ImageSampler(ImageSampler&& o) noexcept: d(o.d) {
	smp = o.smp;
	o.smp = nullptr;
}

ImageSampler::~ImageSampler() {
	if (smp != nullptr) vkDestroySampler(d.d, smp, nullptr);
}

//----------------------------------------------
//------------------FrameBuffer-----------------
//----------------------------------------------

FrameBuffer::FrameBuffer(LogicalDevice& device, RenderPass& rp, uvec2 nSize,
  std::vector<ImageView*> attachments, uint layers):
  d(device) {
	size = nSize;
	std::vector<VkImageView> att;
	att.reserve(attachments.size());
	for (auto& iv : attachments) att.push_back(iv->imgView);
	VkFramebufferCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = rp.rp;
	info.attachmentCount = (uint)att.size();
	info.pAttachments = att.data();
	info.width = size.x;
	info.height = size.y;
	info.layers = layers;
	if (vkCreateFramebuffer(d.d, &info, nullptr, &fb) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}
FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept: d(other.d) {
	fb = other.fb;
	size = other.size;
	other.fb = nullptr;
}
FrameBuffer::~FrameBuffer() {
	if (fb != nullptr) vkDestroyFramebuffer(d.d, fb, nullptr);
}
