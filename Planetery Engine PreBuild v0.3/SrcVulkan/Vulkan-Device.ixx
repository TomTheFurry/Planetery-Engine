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
	/// @addtogroup vkDevice Device
	/// @ingroup vulkan
	/// A group of vulkan classes that is related to device.
	/// @{

	/// <summary>
	/// Surface minimization is detected
	/// </summary>
	/// @todo move to Swapchain module
	struct SurfaceMinimizedException {};

	/// <summary>
	/// Vulkan API Layer
	/// </summary>
	/// @todo Move to new Extension module
	struct Layer {
		const char* name;
		uint version;
	};
	/// <summary>
	/// Vulkan API Extension
	/// </summary>
	/// @todo Move to new Extension module
	struct Extension {
		const char* name;
		uint version;
	};
	// TODO: Currently PhysicalDevice is more like pointer/cache data for a
	// physical device. Maybe fix that via splitting physicalDevice &
	// physicalDeviceProperties Or maybe change that to PhysicalDeviceHandle...?

	/// <summary>
	/// A physical device
	/// </summary>
	/// This is a repesentation of a hardware, like a GPU. It caches the
	/// hardware related informations, used by other classes to properly
	/// construct an object that is compatible with the underlying hardware
	class PhysicalDevice
	{
	  public:
		/** <summary>
		 * Get a usable physical device
		 * </summary>
		 * Calling this function will run a series of check on all avalible
		 * physical devices, and pick the most suitable device. The checks
		 * include:
		 * <ol>
		 * <li>Have default-required and requested Extensions </li>
		 * <li>Have the following Features:
		 *	<ol>
		 *		<li>Geometry Shader Support</li>
		 *		<li>Tesselation Shader Support</li>
		 *		<li>Anisotropic Filtering Support</li>
		 *		<li>
		 * @todo more to be added
		 *
		 * </li>
		 * </ol>
		 * </li>
		 * <li>Meet the following Queue family requirements: <ol>
		 *		<li>At least 1 graphic capable queue family</li>
		 *		<li>
		 * @bug The first graphic capable queue family supports Present
		 * to the optional OSSurface
		 *
		 * </li>
		 * </ol></li>
		 * </ol>
		 *
		 * However, the following properties will effect how suitable a certain
		 * device is:
		 * <ul>
		 * <li>Discrete GPU vs Intergrated GPU</li>
		 * <li>Max work group size for Compute Shaders</li>
		 * <li>Max number of output vertices for Geometry Shaders</li>
		 * <li>Max number of output data for Geometry Shaders</li>
		 * <li>Max memory size for sparse memories
		 * @bug This should be total
		 * VRAM size...
		 *
		 * </li>
		 * <li>
		 * @todo more...
		 *
		 * </li>
		 * </ul>
		 *
		 * <param name="osSurface">The surface that the returned device should
		 * be complitible with \(Optional\)</param>
		 * <returns>A physical device that matches requirements</returns>
		 */
		static PhysicalDevice* getUsablePhysicalDevice(
		  OSRenderSurface* osSurface);
		/// @todo Make this private
		PhysicalDevice(
		  VkPhysicalDevice _d, OSRenderSurface* renderSurface = nullptr);
		/// @todo Make this private
		PhysicalDevice() = default;
		/// <summary>
		/// Copy Constructer
		/// </summary>
		/// @note You are only copying the computed cache of the physical device
		/// properties. Therefore it is safe to be copyed around.
		PhysicalDevice(const PhysicalDevice& other);
		/// <summary>
		/// Move Constructer
		/// </summary>
		/// @warning Only safe if no other objects are using the cache stored in
		/// the object!
		PhysicalDevice(PhysicalDevice&& other) noexcept;
		/// See PhysicalDevice(const PhysicalDevicec&)
		PhysicalDevice& operator=(const PhysicalDevice&) = default;
		/// See PhysicalDevice(PhysicalDevicec&&)
		PhysicalDevice& operator=(PhysicalDevice&&) = default;
		/// @todo Make this private
		VkPhysicalDevice d = nullptr;
		/// @todo Make this private
		VkPhysicalDeviceFeatures2 features10;
		/// @todo Make this private
		VkPhysicalDeviceVulkan11Features features11;
		/// @todo Make this private
		VkPhysicalDeviceVulkan12Features features12;
		/// @todo Make this private
		VkPhysicalDeviceProperties2 properties10;
		/// @todo Make this private
		VkPhysicalDeviceVulkan11Properties properties11;
		/// @todo Make this private
		VkPhysicalDeviceVulkan12Properties properties12;
		/// @todo Make this private
		VkPhysicalDeviceMemoryProperties memProperties;
		/// @todo Make this private
		std::vector<VkQueueFamilyProperties> queueFamilies;
		/// @todo Make this private... Or is this needed?
		OSRenderSurface* renderOut = nullptr;
		/// An arbitrary score for how suitable this physical device is.
		/// See also PhysicalDevice::getUsablePhysicalDevice()
		/// @todo Make this a function
		float rating;
		/// Whether this device meets the basic requirments.
		/// See also PhysicalDevice::getUsablePhysicalDevice()
		/// @todo Make this a function
		bool meetRequirements;
		// TODO: cache the result
		/// Get an index for an avaliable Memory Type that passes the bit filter
		/// and supports certain \c vk::MemoryFeature.
		/// <param name="bitFilter">Memory index that should be filtered
		/// out</param> <param name="feature">Flags of required \c
		/// vk::MemoryFeature</param>
		uint getMemoryTypeIndex(
		  uint bitFilter, Flags<MemoryFeature> feature) const;
		/// Returns the underlying native handle
		/// @todo Make this a function
		VkPhysicalDevice operator->() { return d; }
		/// Returns the underlying native handle
		/// @todo Make this a function
		const VkPhysicalDevice operator->() const { return d; }
		/// Check if the other PhysicalDevice points to the same underlying
		/// physical deivce
		bool operator==(const PhysicalDevice& other) const {
			return d == other.d;
		}
		/// Order/Compare devices according to their score
		std::partial_ordering operator<=>(const PhysicalDevice& other) const {
			return rating <=> other.rating;
		}
		/// Destructor
		~PhysicalDevice() = default;
	};

	/// <summary>
	/// A logical device
	/// </summary>
	/// A logical repesentation of an device. Since an device may be used by
	/// multiple different programs, this repesent the \e program that runs on
	/// the specified PhysicalDevice. It also contains a QueuePool that holds
	/// Queue for submition of commends to the \e program.
	/// @note Almost all vulkan objects store a reference to its LogicalDevice,
	/// as the construction/destruction is handled by the \e program repesented
	/// by the LogicalDevice.
	class LogicalDevice
	{
		util::OptionalUnique<QueuePool> queuePool;

	  public:
		/// @todo Move to private
		void _setup(const QueuePoolLayout& queueLayout);
		/// <summary>
		/// Make a logical device
		/// </summary>
		/// <param name="pd">The physical device it should use</param>
		/// <param name="queueLayout">A QueuePool layout it should try and make.
		/// Error if QueuePoolLayout is invalid with provided physical
		/// device</param>
		LogicalDevice(PhysicalDevice& pd, const QueuePoolLayout& queueLayout);
		LogicalDevice(const LogicalDevice&) = delete;
		/// @note Move ctor is disabled as futher test for other obj handling
		/// moves is needed
		LogicalDevice(LogicalDevice&&) = delete;
		/// @todo Move this private
		VkDevice d;
		/// @todo Move this private & make a func
		std::map<uint, MemoryPool> memoryPools;
		/// <summary>
		/// Allocate a memory section
		/// </summary>
		/// <param name="bitFilter">The memory type to filter out</param>
		/// <param name="feature">The requested memory features</param>
		/// <param name="n">The size of requested memory</param>
		/// <param name="align">The alignment of requiested memory</param>
		/// <returns>
		/// uint: The memoryPool index \n
		/// MemoryPointer: The allocated memory section pointer
		/// </returns>
		std::pair<uint, MemoryPointer> allocMemory(
		  uint bitFilter, Flags<MemoryFeature> feature, size_t n, size_t align);
		/// <summary>
		/// Free a memory section
		/// </summary>
		/// <param name="memoryIndex">The MemoryPool index</param>
		/// <param name="ptr">
		/// The to-be-freed memory section pointer
		/// </param>
		/// @warning Requires that the provided memory section be the same one
		/// that is returned by calling this object's allocMemory(), which has
		/// not been freed yet
		void freeMemory(uint memoryIndex, MemoryPointer ptr);
		/// Get the device's QueuePool
		QueuePool& getQueuePool() { return *queuePool; }
		const QueuePool& getQueuePool() const { return *queuePool; }
		/// @todo remove this
		VkDevice operator->() { return d; }
		/// @todo remove this
		const VkDevice operator->() const { return d; }
		/// Destructor
		~LogicalDevice();
		/// @todo private this and add func
		PhysicalDevice& pd;
	};

	///@}
}
