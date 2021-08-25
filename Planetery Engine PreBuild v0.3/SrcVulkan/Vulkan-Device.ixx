export module Vulkan: Device;
export import: Internal;
import: Memory;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Device class:
export namespace vk {
	struct MinimizedSurfaceException {};

	struct Layer {
		const char* name;
		uint version;
	};
	struct Extension {
		const char* name;
		uint version;
	};
	class PhysicalDevice
	{
	  public:
		static PhysicalDevice getUsablePhysicalDevice(
		  OSRenderSurface* osSurface);
		PhysicalDevice(
		  VkPhysicalDevice _d, OSRenderSurface* renderSurface = nullptr);
		PhysicalDevice() = default;
		PhysicalDevice(const PhysicalDevice& other);
		PhysicalDevice(PhysicalDevice&& other) noexcept;
		PhysicalDevice& operator=(const PhysicalDevice&) = default;
		PhysicalDevice& operator=(PhysicalDevice&&) = default;
		VkPhysicalDevice d = nullptr;
		VkPhysicalDeviceFeatures2 features10;
		VkPhysicalDeviceVulkan11Features features11;
		VkPhysicalDeviceVulkan12Features features12;
		VkPhysicalDeviceProperties2 properties10;
		VkPhysicalDeviceVulkan11Properties properties11;
		VkPhysicalDeviceVulkan12Properties properties12;
		VkPhysicalDeviceMemoryProperties memProperties;
		std::vector<VkQueueFamilyProperties> queueFamilies;
		float rating;
		bool meetRequirements;
		OSRenderSurface* renderOut = nullptr;
		QueueFamilyIndex getQueueFamily(
		  VkQueueFlags requirement, OSRenderSurface* displayOutput = nullptr);
		//TODO: cache the result
		uint getMemoryTypeIndex(
		  uint bitFilter, Flags<MemoryFeature> feature) const;
		LogicalDevice* makeDevice(
		  VkQueueFlags requirement, OSRenderSurface* renderSurface = nullptr) &;
		LogicalDevice* makeDevice(VkQueueFlags requirement,
		  OSRenderSurface* renderSurface = nullptr) &&;
		VkPhysicalDevice operator->() { return d; }
		const VkPhysicalDevice operator->() const { return d; }
		bool operator==(const PhysicalDevice& other) const {
			return d == other.d;
		}
		std::partial_ordering operator<=>(const PhysicalDevice& other) const {
			return rating <=> other.rating;
		}
		SwapChainSupport getSwapChainSupport() const;
		~PhysicalDevice() = default;
	};
	class LogicalDevice
	{
	  public:

		void _setup();
		LogicalDevice(
		  const PhysicalDevice& pd, QueueFamilyIndex queueFamilyIndex);
		LogicalDevice(PhysicalDevice&& pd, QueueFamilyIndex queueFamilyIndex);
		LogicalDevice(const LogicalDevice&) = delete;
		LogicalDevice(LogicalDevice&&) noexcept;
		VkDevice d;
		VkQueue queue;
		QueueFamilyIndex queueIndex;
		//FIXME: LogicalDevice missing getSwapchain() method!
		util::OptionalUniquePtr<SwapChain> swapChain;
		std::vector<CommendPool> commendPools;
		std::map<uint, MemoryPool> memoryPools;
		std::pair<uint, MemoryPointer> allocMemory(
		  uint bitFilter,
		  Flags<MemoryFeature> feature, size_t n, size_t align);
		void freeMemory(uint memoryIndex, MemoryPointer ptr);
		bool isSwapchainValid() const;
		SwapChain& getSwapchain();
		void loadSwapchain(uvec2 preferredSize = uvec2(-1));
		void unloadSwapchain();
		bool isSwapchainLoaded() const;
		CommendPool& getCommendPool(CommendPoolType type);
		CommendBuffer getSingleUseCommendBuffer();
		VkDevice operator->() { return d; }
		const VkDevice operator->() const { return d; }
		~LogicalDevice();
		PhysicalDevice pd;
	};
	class OSRenderSurface
	{
	  public:
		OSRenderSurface();
		OSRenderSurface(const OSRenderSurface&) = delete;
		OSRenderSurface(OSRenderSurface&&) = delete;
		VkSurfaceKHR surface = nullptr;
		VkSurfaceKHR operator->() { return surface; }
		const VkSurfaceKHR operator->() const { return surface; }
		~OSRenderSurface();
	};
	class SwapChainSupport
	{
	  public:
		SwapChainSupport() = default;
		SwapChainSupport(
		  const PhysicalDevice& pd, const OSRenderSurface& surface);
		VkSurfaceFormatKHR getFormat() const;
		VkPresentModeKHR getPresentMode(bool preferRelaxedVBlank = false) const;
		uvec2 getSwapChainSize(uvec2 preferredSize = uvec2(uint(-1))) const;
		uint getImageCount(uint preferredCount = uint(4)) const;
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	class SwapChain
	{
		//TODO: Add support for setting preferred Swapchain Method
		void _build(uvec2 preferredSize, uint preferredImageCount,
		  WindowTransparentType transparentWindowType);
	  public:
		static void setCallback(SwapchainCallback callback);
		SwapChain(const OSRenderSurface& surface, LogicalDevice& device,
		  uvec2 preferredSize, uint preferredImageCount = uint(4),
		  WindowTransparentType transparentWindowType = WindowTransparentType::RemoveAlpha);
		SwapChain(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		void rebuild(uvec2 preferredSize, uint preferredImageCount = uint(4),
		  WindowTransparentType transparentWindowType =
			WindowTransparentType::PreMultiplied);
		uint getImageCount() const;
		RenderTick& renderNextFrame();
		void markAllTickAsOutdated();
		bool isValid() const;
		LogicalDevice& d;
		const OSRenderSurface& sf;
		VkSwapchainKHR sc;
		VkSurfaceFormatKHR surfaceFormat;
		uvec2 pixelSize;
		std::vector<VkImage> swapChainImages;
		std::vector<util::OptionalUniquePtr<RenderTick>> ticks;
		bool outdated = false;
		ImageView getChainImageView(uint index);
		~SwapChain();
		//std::pair<uint, Semaphore> getNextImage();
	};
}
