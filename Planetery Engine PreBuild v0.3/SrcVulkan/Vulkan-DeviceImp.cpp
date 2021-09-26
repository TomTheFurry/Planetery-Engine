module Vulkan: DeviceImp;
import: Device;
import: Swapchain;
import: Enum;
import: Commend;
import: Image;
import std.core;
import Define;
import ThreadEvents;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
import "GlfwModule.h";
using namespace vk;

PhysicalDevice* PhysicalDevice::getUsablePhysicalDevice(
  OSRenderSurface* osSurface) {
	std::vector<VkPhysicalDevice> pDevices;
	logger.newLayer();
	logger << "Vulkan: Scanning Graphic Cards...\n";
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(getVkInstance(), &deviceCount, nullptr);
		if (deviceCount == 0) {
			logger("Vulkan did not find any available graphic cards!\n");
			logger.closeLayer();
			return nullptr;
		}
		pDevices.resize(deviceCount);
		vkEnumeratePhysicalDevices(
		  getVkInstance(), &deviceCount, pDevices.data());
	}
	std::vector<util::OptionalUniquePtr<PhysicalDevice>> objDevices;
	objDevices.reserve(pDevices.size());
	for (auto& dPtr : pDevices) {
		auto& dOpPtr = objDevices.emplace_back(new PhysicalDevice(dPtr, osSurface));
		if (!dOpPtr->meetRequirements)
			objDevices.pop_back();
	}
	if (objDevices.empty()) {
		logger("Vulkan did not find any usable graphic cards!\n");
		logger.closeLayer();
		return nullptr;
	}
	logger.closeLayer();
	return std::max_element(objDevices.begin(), objDevices.end(),
	  [](const auto& p1, const auto& p2) { return *p1 < *p2; })->getOwnership();
}

PhysicalDevice::PhysicalDevice(
  VkPhysicalDevice _d, OSRenderSurface* renderSurface) {
	d = _d;
	features10 = {};
	features11 = {};
	features12 = {};
	properties10 = {};
	properties11 = {};
	properties12 = {};
	features10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	features10.pNext = &features11;
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features11.pNext = &features12;
	properties10.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties11.sType =
	  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
	properties10.pNext = &properties11;
	properties12.sType =
	  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
	properties11.pNext = &properties12;
	logger.newLayer();
	vkGetPhysicalDeviceProperties2(d, &properties10);
	vkGetPhysicalDeviceFeatures2(d, &features10);
	uint queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, nullptr);
	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
	  d, &queueFamilyCount, queueFamilies.data());
	vkGetPhysicalDeviceMemoryProperties(d, &memProperties);
	renderOut = renderSurface;
	auto& limit = properties10.properties.limits;
	logger << properties10.properties.deviceName << "...";

	// ----Starting checks for requirements----
	meetRequirements = true;
	{  // Check extensions support
		uint extensionCount;
		vkEnumerateDeviceExtensionProperties(
		  d, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
		  d, nullptr, &extensionCount, availableExtensions.data());

		// OPTI: change it so that it sorts the avilb' extension before find
		// matches using std::lower_bound.
		for (const char* neededExt : getRequestedDeviceExtensions()) {
			if (*neededExt == '\0') continue;
			if (std::find_if(availableExtensions.begin(),
				  availableExtensions.end(),
				  [neededExt](const VkExtensionProperties& p) {
					  return std::strcmp(neededExt, p.extensionName) == 0;
				  })
				== availableExtensions.end()) {
				logger << "Failed check: Missing requested device extension: "
					   << neededExt << "\n";
				meetRequirements = false;
				continue;
			}
		}
	}
	if (!features10.features.geometryShader) {
		logger << "Failed check: No support for geometry shader\n";
		meetRequirements = false;
	}
	if (!features10.features.tessellationShader) {
		logger << "Failed check: No support for tessellation shader\n";
		meetRequirements = false;
	}
	if (!features10.features.samplerAnisotropy) {
		logger << "Failed check: No support for anisotropic filtering\n";
		meetRequirements = false;
	}

	// OPTI: Currently querying the queue support, and dump it afterwards.
	// Is there some way to push that out for reusing when making the
	// logical device? Maybe split up QueueSupport and QueuePoolLayout?
	QueuePoolLayout qpl{*this};
	uint graphicsQueue = qpl.findFamilyBySupportType(QueueType::Graphics);
	if (graphicsQueue == uint(-1)) {
		logger << "Failed check: No queue supports Graphics processing\n";
		meetRequirements = false;
	} else if (renderSurface != nullptr
			   && !qpl.checkQueuePresentSupport(graphicsQueue, *renderSurface)) {
		// FIXME: Currently only cares about first grapics queue support for
		// presentation
		logger << "Failed check: First Graphics queue " << graphicsQueue << " does not support "
				  "presenting to target OSRenderSurface\n";
		meetRequirements = false;
	}

	// ----Ending checks for requirements----
	if (meetRequirements) {
		logger << " Usable.";
	} else {
		logger << " Failed to meet Min Requirements.";
	}

	// GPU Ratings
	logger << " Rating: ";
	rating = 0;
	if (properties10.properties.deviceType
		== VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		rating += 1000;
	if (properties10.properties.deviceType
		== VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		rating += 100;
	rating += float(std::log(double(limit.maxComputeWorkGroupSize[0])
							 * double(limit.maxComputeWorkGroupSize[1])
							 * double(limit.maxComputeWorkGroupSize[2]))
					* 100.);
	rating += limit.maxGeometryOutputVertices;
	rating += limit.maxGeometryTotalOutputComponents;
	rating += float(std::log(double(limit.sparseAddressSpaceSize)) * 100.);

	logger << std::to_string(rating) << "\n";
	logger.closeLayer();
}

PhysicalDevice::PhysicalDevice(const PhysicalDevice& o):
  PhysicalDevice(o.d, o.renderOut){};

PhysicalDevice::PhysicalDevice(PhysicalDevice&& o) noexcept:
  queueFamilies(std::move(o.queueFamilies)) {
	d = o.d;
	features10 = o.features10;
	features11 = o.features11;
	features12 = o.features12;
	properties10 = o.properties10;
	properties11 = o.properties11;
	properties12 = o.properties12;
	features10.pNext = &features11;
	features11.pNext = &features12;
	properties10.pNext = &properties11;
	properties11.pNext = &properties12;
	meetRequirements = o.meetRequirements;
	rating = o.rating;
	renderOut = o.renderOut;
	memProperties = o.memProperties;
	o.d = nullptr;
}

uint PhysicalDevice::getMemoryTypeIndex(
  uint bitFilter, Flags<MemoryFeature> feature) const {
	uint requirement = 0;
	if (feature.has(MemoryFeature::Mappable)) {
		requirement |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		if (feature.has(MemoryFeature::Coherent)) {
			requirement |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		} else {
			requirement |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
		}
	} else {
		requirement |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}
	for (uint i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((bitFilter & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & requirement)
				 == requirement) {
			return i;
		}
	}
	return uint(-1);
}

void LogicalDevice::_setup(const QueuePoolLayout& queueLayout) {
	auto qInfos = queueLayout.getQueueCreateInfos();
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = &pd.features10;
	deviceInfo.pQueueCreateInfos = qInfos.data();
	deviceInfo.queueCreateInfoCount = qInfos.size();
	deviceInfo.enabledExtensionCount =
	  (uint)std::size(getRequestedDeviceExtensions());
	deviceInfo.ppEnabledExtensionNames =
	  std::data(getRequestedDeviceExtensions());
	deviceInfo.pEnabledFeatures = NULL;
	deviceInfo.flags = 0;
	if (vkCreateDevice(pd.d, &deviceInfo, nullptr, &d) != VK_SUCCESS) {
		logger("Vulkan failed to create main graphical device!\n");
		throw "VulkanCreateGraphicalDeviceFailure";
	}
	queuePool.make(queueLayout, *this);
}

LogicalDevice::LogicalDevice(
  PhysicalDevice& d, const QueuePoolLayout& queueLayout):
  pd(d) {
	_setup(queueLayout);
}
// FIXME: Currently disabled LogicalDevice move as most objects CANNOT handle
// it!
/*
LogicalDevice::LogicalDevice(LogicalDevice&& o) noexcept:
  pd(std::move(o.pd)), commendPools(std::move(o.commendPools)) {
	queue = o.queue;
	queueIndex = o.queueIndex;
	d = o.d;
	o.d = nullptr;
}*/
LogicalDevice::~LogicalDevice() {
	if (swapChain) swapChain.reset();
	queuePool.reset();
	if (d) vkDestroyDevice(d, nullptr);
}
std::pair<uint, MemoryPointer> LogicalDevice::allocMemory(
  uint bitFilter, Flags<MemoryFeature> feature, size_t n, size_t align) {
	uint key = pd.getMemoryTypeIndex(bitFilter, feature);
	auto iter = memoryPools.lower_bound(key);
	if (iter == memoryPools.end() || iter->first != key)
		iter = memoryPools.emplace_hint(
		  iter, key, MemoryPool(65536, MemoryAllocator(*this, key)));
	return std::pair(key, iter->second.alloc(n, align));
}
void LogicalDevice::freeMemory(uint memoryIndex, MemoryPointer ptr) {
	// logger("Vulkan Freeing memId: ", memoryIndex);
	memoryPools.at(memoryIndex).free(ptr);
}
/*
bool LogicalDevice::isSwapchainValid() const {
	return swapChain && swapChain->isValid();
}


Swapchain& LogicalDevice::getSwapchain() { return *swapChain; }

bool LogicalDevice::loadSwapchain(uvec2 size) {
	assert(pd.renderOut != nullptr);
	try {
		if (swapChain) swapChain->rebuild(size, PREFERRED_IMAGE_COUNT);
		else
			swapChain.make(*pd.renderOut, *this, size, PREFERRED_IMAGE_COUNT);
		return true;
	} catch (SurfaceMinimizedException) { return false; }
}
void LogicalDevice::unloadSwapchain() { swapChain.reset(); }
bool LogicalDevice::isSwapchainLoaded() const { return swapChain; }
*/

/*
CommendBuffer LogicalDevice::getSingleUseCommendBuffer() {
	auto& cp = getCommendPool(CommendPoolType::Shortlived);
	return cp.makeCommendBuffer();
}*/