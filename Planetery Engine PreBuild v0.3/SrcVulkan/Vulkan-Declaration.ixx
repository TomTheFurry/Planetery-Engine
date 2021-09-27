export module Vulkan: Declaration;
import: Enum;
import "VulkanExtModule.h";
import std.core;
import Define;
import Util;

// Class Declaration:
export namespace vk {
	// Basics
	using NativeHandle = size_t;

	struct ComplexObject {
		ComplexObject() = default;
		// ComplexObject(ComplexObject&&) = delete;
		// ComplexObject(const ComplexObject&) = default;
		virtual ~ComplexObject() = default;
	};
	template<typename Class>
	concept ComplexObjects = std::is_base_of_v<ComplexObject, Class>;

	// Lifetime class:
	class LifetimeManager;
	class MonotonicLifetimeManager;

	// TODO: Extension classes: Seperate it from Device class

	// Device classes:
	struct Layer;
	struct Extension;
	typedef uint QueueFamilyIndex;
	class PhysicalDevice;
	class LogicalDevice;
	struct DeviceCallback {
		void (*onCreate)(LogicalDevice&) = nullptr;
		void (*onDestroy)(LogicalDevice&) = nullptr;
	};
	class OSRenderSurface;

	// Queue classes:
	class QueuePoolLayout;
	class QueuePool;
	class Queue;

	// Swapchain class:
	class Swapchain;
	struct SwapchainCallback {
		void (*onCreate)(Swapchain&, bool) = nullptr;
		void (*onDestroy)(Swapchain&, bool) = nullptr;
		void (*onSurfaceMinimized)(Swapchain&) = nullptr;
	};
	class SwapchainImage;
	struct FrameCallback {
		void (*onCreate)(SwapchainImage&) = nullptr;
		void (*onDraw)(SwapchainImage&) = nullptr;
		void (*onDestroy)(SwapchainImage&) = nullptr;
	};

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
	class RenderPipeline;

}

// Internal Functions:
export namespace vk {
	std::span<const Layer> getRequestedLayers();
	std::span<const Extension> getRequestedExtensions();
	std::span<const char* const> getRequestedDeviceExtensions();
	VkInstance getVkInstance();
}
