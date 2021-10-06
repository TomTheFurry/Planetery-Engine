export module Vulkan: Device;
export import: Declaration;
import: Memory;
import: Enum;
import: Queue;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Device class:

export namespace vk {
	/// @addtogroup vkDevice Devices
	/// @ingroup vulkan
	/// @{
    

	struct SurfaceMinimizedException {};

	struct Layer {
		const char* name;
		uint version;
	};
	struct Extension {
		const char* name;
		uint version;
	};
	// TODO: Currently PhysicalDevice is more like pointer/cache data for a
	// physical device. Maybe fix that via splitting physicalDevice &
	// physicalDeviceProperties Or maybe change that to PhysicalDeviceHandle...?
	class PhysicalDevice
	{
	  public:
		static PhysicalDevice* getUsablePhysicalDevice(
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
		// TODO: cache the result
		uint getMemoryTypeIndex(
		  uint bitFilter, Flags<MemoryFeature> feature) const;
		VkPhysicalDevice operator->() { return d; }
		const VkPhysicalDevice operator->() const { return d; }
		bool operator==(const PhysicalDevice& other) const {
			return d == other.d;
		}
		std::partial_ordering operator<=>(const PhysicalDevice& other) const {
			return rating <=> other.rating;
		}
		~PhysicalDevice() = default;
	};

	class LogicalDevice
	{
		util::OptionalUnique<QueuePool> queuePool;

	  public:
		void _setup(const QueuePoolLayout& queueLayout);
		LogicalDevice(PhysicalDevice& pd, const QueuePoolLayout& queueLayout);
		LogicalDevice(const LogicalDevice&) = delete;
		// FIXME: Currently disabled LogicalDevice move as most objects CANNOT
		// handle it!
		// LogicalDevice(LogicalDevice&&) noexcept;
		LogicalDevice(LogicalDevice&&) = delete;
		VkDevice d;
		std::map<uint, MemoryPool> memoryPools;
		std::pair<uint, MemoryPointer> allocMemory(
		  uint bitFilter, Flags<MemoryFeature> feature, size_t n, size_t align);
		void freeMemory(uint memoryIndex, MemoryPointer ptr);

		QueuePool& getQueuePool() { return *queuePool; }
		const QueuePool& getQueuePool() const { return *queuePool; }
		VkDevice operator->() { return d; }
		const VkDevice operator->() const { return d; }
		~LogicalDevice();
		PhysicalDevice& pd;
	};
	
	///@}
}
