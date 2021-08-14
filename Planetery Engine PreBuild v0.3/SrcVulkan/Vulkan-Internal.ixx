export module Vulkan: Internal;
import: Enum;
import "VulkanExtModule.h";
import std.core;
import Define;
import Util;

// Class Declaration:
export namespace vk {
	// Device class:
	struct Layer;
	struct Extension;
	typedef uint QueueFamilyIndex;
	class PhysicalDevice;
	class LogicalDevice;
	class OSRenderSurface;
	class SwapChainSupport;
	class SwapChain;
	// Memory class:
	struct DeviceMemory;
	struct MemoryPointer;
	class MemoryPool;
	class MemoryAllocator;
	// Buffer class:
	class Buffer;
	class VertexBuffer;
	class IndexBuffer;
	class UniformBuffer;
	// Image class:
	class Image;
	class ImageView;
	class ImageSampler;
	class FrameBuffer;
	// Commend class:
	class CommendPool;
	class CommendBuffer;
	// Sync class:
	struct SyncLine;
	struct SyncPoint;
	typedef ulint SyncNumber;
	class Fence;
	class Semaphore;
	class TimelineSemaphore;
	// Descriptor class:
	struct DescriptorLayoutBinding;
	class DescriptorLayout;
	class DescriptorPool;
	class DescriptorContainer;
	class DescriptorSet;
	// Shader class:
	class ShaderCompiled;
	// Pipeline class:
	class VertexAttribute;
	class RenderPass;
	class ShaderPipeline;
	// Tick class:
	class RenderTick;
}

// Internal Functions:
export namespace vk {
	std::span<const Layer> getRequestedLayers();
	std::span<const Extension> getRequestedExtensions();
	std::span<const char* const> getRequestedDeviceExtensions();
	VkInstance getVkInstance();
}


