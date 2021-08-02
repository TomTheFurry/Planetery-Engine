module;
#include "Marco.h"
#pragma warning(disable : 26812)
#include <vulkan/vulkan.h>
#include <assert.h>
module Vulkan;
import: Internal;
import: Enum;
import std.core;
import Define;
import Logger;
using namespace vk;

size_t formatUnitSize(VkFormat format) {
	switch (format) {
	case VK_FORMAT_R32_SFLOAT: return sizeof(float);
	case VK_FORMAT_R32G32_SFLOAT: return sizeof(vec2);
	case VK_FORMAT_R32G32B32_SFLOAT: return sizeof(vec3);
	case VK_FORMAT_R32G32B32A32_SFLOAT: return sizeof(vec4);
	};
	throw "VulkanInvalidFormat";
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

Image::Image(LogicalDevice& d, uvec3 texSize, uint texDimension,
  VkFormat texFormat, Flags<TextureUseType> texUsage,
  TextureActiveUseType startingUsage, Flags<MemoryFeature> texMemFeature,
  Flags<TextureFeature> texFeature, uint mipLevels, uint layers,
  uint subsamples):
  d(d) {
	if (texMemFeature.has(MemoryFeature::Mappable)) {
		if (texMemFeature.has(MemoryFeature::Coherent))
			mappingMinAlignment =
			  d.pd.properties10.properties.limits.nonCoherentAtomSize;
		else
			mappingMinAlignment =
			  d.pd.properties10.properties.limits.minMemoryMapAlignment;
	} else {
		mappingMinAlignment = size_t(-1);
	}
	if (texMemFeature.has(MemoryFeature::IndirectWritable)) {
		texUsage.set(TextureUseType::TransferDst);
	}
	if (texMemFeature.has(MemoryFeature::IndirectReadable)) {
		texUsage.set(TextureUseType::TransferSrc);
	}

	size = texSize;
	feature = texFeature;
	memFeature = texMemFeature;
	usage = texUsage;
	dimension = texDimension;
	activeUsage = startingUsage;
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
	iInfo.tiling = mappingMinAlignment == uint(-1) ? VK_IMAGE_TILING_OPTIMAL
												   : VK_IMAGE_TILING_LINEAR;
	iInfo.usage = (VkImageUsageFlags)texUsage;
	iInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// iInfo.queueFamilyIndexCount;
	// iInfo.pQueueFamilyIndices;
	iInfo.initialLayout = (VkImageLayout)startingUsage;

	if (vkCreateImage(d.d, &iInfo, nullptr, &img) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	// Alloc Memory
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(d.d, img, &memRequirements);
	uint memTypeIndex =
	  d.pd.getMemoryTypeIndex(memRequirements.memoryTypeBits, texMemFeature);
	// TODO: Add fallback
	if (memTypeIndex == -1) throw "VulkanFailedToGetMemoryType";
	// TODO: Fix allocator for custom allocation in PhysicalDevice
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = memTypeIndex;
	if (vkAllocateMemory(d.d, &allocInfo, nullptr, &dm) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindImageMemory(d.d, img, dm, 0);
}
Image::Image(Image&& other) noexcept: d(other.d) {
	img = other.img;
	other.img = nullptr;
	dm = other.dm;
	other.dm = nullptr;
}
Image::~Image() {
	if (img != nullptr) vkDestroyImage(d.d, img, nullptr);
	if (dm != nullptr) vkFreeMemory(d.d, dm, nullptr);
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
	vkMapMemory(d.d, dm, 0, VK_WHOLE_SIZE, 0, &mappedPtr);
	return mappedPtr;
}
void* Image::map(size_t nSize, size_t offset) {
	if constexpr (DO_SAFETY_CHECK) {
		if (nSize + offset > texMemorySize) throw "VulkanImageOutOfRange";
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr != nullptr) throw "VulkanImageAlreadyMapped";
		if (nSize % mappingMinAlignment != 0)
			throw "VulkanImageNotOnMinAlignment";
		if (offset % mappingMinAlignment != 0)
			throw "VulkanImageNotOnMinAlignment";
		if (activeUsage != TextureActiveUseType::General
			&& activeUsage != TextureActiveUseType::HostWritable)
			throw "VulkanImageIncorrectActiveUsage";
	}
	vkMapMemory(d.d, dm, offset, nSize, 0, &mappedPtr);
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
	r.memory = dm;
	r.offset = 0;
	r.size = VK_WHOLE_SIZE;
	vkFlushMappedMemoryRanges(d.d, 1, &r);
}
void Image::flush(size_t nSize, size_t offset) {
	if constexpr (DO_SAFETY_CHECK) {
		if (nSize + offset > texMemorySize) throw "VulkanImageOutOfRange";
		if (!memFeature.has(MemoryFeature::Mappable))
			throw "VulkanImageNotMappable";
		if (mappedPtr == nullptr) throw "VulkanImageNotMapped";
		if (nSize % mappingMinAlignment != 0)
			throw "VulkanImageNotOnMinAlignment";
		if (offset % mappingMinAlignment != 0)
			throw "VulkanImageNotOnMinAlignment";
	}
	VkMappedMemoryRange r{};
	r.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	r.memory = dm;
	r.size = nSize;
	r.offset = offset;
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
	vkUnmapMemory(d.d, dm);
}




void Image::blockingIndirectWrite(const void* data) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!memFeature.has(MemoryFeature::IndirectWritable))
			throw "VulkanBufferNotIndirectWritable";
	}
	auto sg = Buffer(d, texMemorySize,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
	sg.directWrite(data);
	auto cb = CommendBuffer(d.getCommendPool(CommendPoolType::Shortlived));
	cb.startRecording(CommendBufferUsage::Streaming);

	cb.cmdCopy(sg, *this, TextureAspect::Color, size, size);

	cb.endRecording();
	VkFenceCreateInfo fInfo{};
	fInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(d.d, &fInfo, nullptr, &fence);
	VkSubmitInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sInfo.waitSemaphoreCount = 0;
	sInfo.commandBufferCount = 1;
	sInfo.pCommandBuffers = &cb.cb;
	sInfo.signalSemaphoreCount = 0;
	vkQueueSubmit(d.queue, 1, &sInfo, fence);
	vkWaitForFences(d.d, 1, &fence, VK_TRUE, -1);
	vkDestroyFence(d.d, fence, nullptr);
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
	auto cb = CommendBuffer(d.getCommendPool(CommendPoolType::Shortlived));
	cb.startRecording(CommendBufferUsage::Streaming);

	cb.cmdCopy(sg, *this, TextureAspect::Color, size, size);

	cb.endRecording();
	VkFenceCreateInfo fInfo{};
	fInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(d.d, &fInfo, nullptr, &fence);
	VkSubmitInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sInfo.waitSemaphoreCount = 0;
	sInfo.commandBufferCount = 1;
	sInfo.pCommandBuffers = &cb.cb;
	sInfo.signalSemaphoreCount = 0;
	vkQueueSubmit(d.queue, 1, &sInfo, fence);
	vkWaitForFences(d.d, 1, &fence, VK_TRUE, -1);
	vkDestroyFence(d.d, fence, nullptr);
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

void vk::Image::blockingTransformActiveUsage(TextureActiveUseType targetUsage) {
	if constexpr (DO_SAFETY_CHECK) {
		//??? What to check?
	}
	auto cb = CommendBuffer(d.getCommendPool(CommendPoolType::Shortlived));
	cb.startRecording(CommendBufferUsage::Streaming);

	cb.cmdChangeState(*this, targetUsage, PipelineStage::TopOfPipe,
	  MemoryAccess::None, PipelineStage::BottomOfPipe, MemoryAccess::None);

	cb.endRecording();
	VkFenceCreateInfo fInfo{};
	fInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	VkFence fence;
	vkCreateFence(d.d, &fInfo, nullptr, &fence);
	VkSubmitInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sInfo.waitSemaphoreCount = 0;
	sInfo.commandBufferCount = 1;
	sInfo.pCommandBuffers = &cb.cb;
	sInfo.signalSemaphoreCount = 0;
	vkQueueSubmit(d.queue, 1, &sInfo, fence);
	vkWaitForFences(d.d, 1, &fence, VK_TRUE, -1);
	vkDestroyFence(d.d, fence, nullptr);
}

