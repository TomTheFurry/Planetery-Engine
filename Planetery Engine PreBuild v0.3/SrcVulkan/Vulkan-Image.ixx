export module Vulkan: Image;
export import: Internal;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Image class:
export namespace vk {
	class Image
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
		void blockingIndirectWrite(const void* data);
		void blockingIndirectWrite(
		  size_t size, size_t offset, const void* data);
		void directWrite(const void* data);
		void directWrite(size_t size, size_t offset, const void* data);

		void blockingTransformActiveUsage(TextureActiveUseType targetUsage);

		// TODO: Check if below is okay with vkMinAlignment
		size_t mappingMinAlignment;	 // size_t(-1) equals no mapping
		size_t texMemorySize;
		uvec3 size;
		uint dimension;
		uint mipLevels;
		uint layers;
		VkFormat format;
		Flags<MemoryFeature> memFeature;
		Flags<TextureFeature> feature;
		Flags<TextureUseType> usage;
		TextureActiveUseType activeUsage;
		VkImage img = nullptr;
		VkDeviceMemory dm = nullptr;
		void* mappedPtr = nullptr;
		LogicalDevice& d;
	};
	class ImageView
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
	class ImageSampler
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
	class FrameBuffer
	{
	  public:
		FrameBuffer(LogicalDevice& device, RenderPass& rp, uvec2 nSize,
		  std::vector<ImageView*> attachments, uint layers = 1);
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer&& other) noexcept;
		~FrameBuffer();
		uvec2 size;
		VkFramebuffer fb = nullptr;
		LogicalDevice& d;
	};
}
