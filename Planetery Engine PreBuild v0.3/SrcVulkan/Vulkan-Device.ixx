export module Vulkan: Device;
export import: Internal;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Device class:
export namespace vk {
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
		std::unique_ptr<SwapChain> swapChain;
		std::vector<CommendPool> commendPools;
		void makeSwapChain(uvec2 size = uvec2(-1));
		void remakeSwapChain(uvec2 size = uvec2(-1));
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
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	class SwapChain
	{
	  public:
		SwapChain(const OSRenderSurface& surface, LogicalDevice& device,
		  uvec2 preferredSize, bool transparentWindow = false);
		SwapChain(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		bool rebuildSwapChain(
		  uvec2 preferredSize, bool transparentWindow = false);
		LogicalDevice& d;
		const OSRenderSurface& sf;
		VkSwapchainKHR sc;
		VkSurfaceFormatKHR surfaceFormat;
		uvec2 pixelSize;
		std::vector<VkImage> swapChainImages;
		ImageView getChainImageView(uint index);
		~SwapChain();
	};
}
