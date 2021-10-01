export module Vulkan: Image;
export import: Declaration;
import: Enum;
import: Memory;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Image class:
export namespace vk {
	// Note: It's starting ActiveUsage is HostWritable if it has
	// MemoryFeature::Mappable, or else Undefined
	class Image: public ComplexObject
	{
	  public:
		// Super ctor
		Image(LogicalDevice& d, uvec3 texSize, uint texDimension,
		  VkFormat texFormat,
		  Flags<TextureUseType> texUsage = TextureUseType::None,
		  Flags<MemoryFeature> texMemFeature = MemoryFeature::None,
		  Flags<TextureFeature> texFeature = TextureFeature::None,
		  uint mipLevels = 1, uint layers = 1, uint subsamples = 1);
		Image(const Image&) = delete;
		Image(Image&& other) noexcept;
		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = default;
		~Image();

		void* map();
		void* map(size_t size, size_t offset);
		void flush();
		void flush(size_t size, size_t offset);
		void unmap();
		void blockingIndirectWrite(
		  ImageActiveUsage usage, TextureAspect targetAspect, const void* data);
		void blockingIndirectWrite(ImageActiveUsage usage,
		  TextureSubLayers layers, uvec3 copyRegion, ivec3 copyOffset,
		  uvec3 inputTextureSize, const void* data);
		// Unsafe
		void directWrite(const void* data);
		// Unsafe
		void directWrite(size_t size, size_t offset, const void* data);

		void blockingTransformActiveUsage(ImageActiveUsage start,
		  ImageActiveUsage end, TextureSubRegion subRegion);

		size_t texMemorySize;
		size_t mSize;
		uvec3 size;
		size_t minAlignment;
		uint dimension;
		uint mipLevels;
		uint layers;
		VkFormat format;
		Flags<MemoryFeature> memFeature;
		Flags<TextureFeature> feature;
		Flags<TextureUseType> usage;
		VkImage img = nullptr;
		uint memoryIndex;
		MemoryPointer ptr;
		void* mappedPtr = nullptr;
		LogicalDevice& d;
	};
	class ImageView: public ComplexObject
	{
	  public:
		ImageView(LogicalDevice& d, const Image& img);
		ImageView(LogicalDevice& d, VkImageViewCreateInfo createInfo);
		ImageView(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		ImageView& operator=(const ImageView&) = delete;
		ImageView& operator=(ImageView&&) = default;
		~ImageView();
		VkImageView imgView = nullptr;
		LogicalDevice& d;
	};
	class ImageSampler: public ComplexObject
	{
	  public:
		ImageSampler(LogicalDevice& d, SamplerFilter minFilter,
		  SamplerFilter magFilter,
		  SamplerClampMode clampModeX = SamplerClampMode::BorderColor,
		  SamplerClampMode clampModeY = SamplerClampMode::BorderColor,
		  SamplerClampMode clampModeZ = SamplerClampMode::BorderColor,
		  SamplerBorderColor borderColor =
			SamplerBorderColor::FloatTransparentBlack,
		  bool normalized = true,
		  SamplerMipmapMode mipmapMode = SamplerMipmapMode::Linear,
		  float lodBias = 0, float minLod = 0, float maxLod = 0);
		ImageSampler(const ImageSampler&) = delete;
		ImageSampler(ImageSampler&& other) noexcept;
		ImageSampler& operator=(const ImageSampler&) = delete;
		ImageSampler& operator=(ImageSampler&&) = default;
		~ImageSampler();

		VkSampler smp = nullptr;
		LogicalDevice& d;
	};
	class FrameBuffer: public ComplexObject
	{
		void _ctor(RenderPass& rp,
		  std::span<const Ref<const ImageView>> attachments, uint layers);

	  public:
		FrameBuffer(LogicalDevice& device, RenderPass& rp, uvec2 nSize,
		  std::span<const Ref<const ImageView>> attachments, uint layers = 1);
		FrameBuffer(LogicalDevice& device, RenderPass& rp, uvec2 nSize,
		  std::initializer_list<Ref<const ImageView>> attachments,
		  uint layers = 1);
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer&& other) noexcept;
		~FrameBuffer();
		uvec2 size;
		VkFramebuffer fb = nullptr;
		LogicalDevice& d;
	};
}
