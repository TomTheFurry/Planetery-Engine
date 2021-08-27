module Vulkan: DeviceImp;
import: Device;
import: Enum;
import: Commend;
import: Image;
import: Tick;
import std.core;
import Define;
import ThreadEvents;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
import "GlfwModule.h";
using namespace vk;

#define PREFERRED_IMAGE_COUNT 18u
#define USE_MAILBOX_MODE
#define PREFERRED_WIN_ALPHA_MODE

PhysicalDevice PhysicalDevice::getUsablePhysicalDevice(
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
	std::vector<PhysicalDevice> objDevices;
	objDevices.reserve(pDevices.size());
	for (auto& dPtr : pDevices) {
		if (!objDevices.emplace_back(dPtr, osSurface).meetRequirements)
			objDevices.pop_back();
	}
	if (objDevices.empty()) {
		logger("Vulkan did not find any usable graphic cards!\n");
		logger.closeLayer();
		return nullptr;
	}
	logger.closeLayer();
	return std::move(*std::max_element(objDevices.begin(), objDevices.end(),
	  [](const auto& p1, const auto& p2) { return p1 < p2; }));
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
	meetRequirements = true;
	// GPU Requirements
	if (!features10.features.geometryShader) meetRequirements = false;
	if (!features10.features.tessellationShader) meetRequirements = false;
	if (!features10.features.samplerAnisotropy) meetRequirements = false;
	{
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
				meetRequirements = false;
				break;
			}
		}
	}
	if (renderSurface != nullptr) {
		if (getQueueFamily(VK_QUEUE_GRAPHICS_BIT, renderSurface) == uint(-1))
			meetRequirements = false;
		auto swapChain = SwapChainSupport(*this, *renderSurface);
		if (swapChain.formats.empty() || swapChain.presentModes.empty())
			meetRequirements = false;
	}

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

QueueFamilyIndex PhysicalDevice::getQueueFamily(
  VkQueueFlags requirement, OSRenderSurface* surfaceOut) {
	for (uint i = 0; i < queueFamilies.size(); i++) {
		auto& q = queueFamilies[i];
		if (((!q.queueFlags) & requirement) == 0) {
			if (surfaceOut == nullptr) return i;
			VkBool32 canRenderToSurface = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(
			  d, i, surfaceOut->surface, &canRenderToSurface);
			if (canRenderToSurface) return i;
		}
	}
	return uint(-1);
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
LogicalDevice* PhysicalDevice::makeDevice(
  VkQueueFlags requirement, OSRenderSurface* renderSurface) & {
	QueueFamilyIndex i = getQueueFamily(requirement, renderSurface);
	if (i == uint(-1)) return nullptr;
	return new LogicalDevice(*this, i);
}
LogicalDevice* PhysicalDevice::makeDevice(
  VkQueueFlags requirement, OSRenderSurface* renderSurface) && {
	QueueFamilyIndex i = getQueueFamily(requirement, renderSurface);
	if (i == uint(-1)) return nullptr;
	return new LogicalDevice(std::move(*this), i);
}
SwapChainSupport PhysicalDevice::getSwapChainSupport() const {
	return SwapChainSupport(*this, *renderOut);
}

void LogicalDevice::_setup() {
	const float One = 1.0f;
	std::array<VkDeviceQueueCreateInfo, 1> queueInfo{};
	queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[0].queueFamilyIndex = queueIndex;
	queueInfo[0].queueCount = 2;
	queueInfo[0].pQueuePriorities = &One;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = &pd.features10;
	deviceInfo.pQueueCreateInfos = queueInfo.data();
	deviceInfo.queueCreateInfoCount = 1;
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
	vkGetDeviceQueue(d, queueIndex, 0, &queue);
	commendPools.reserve(static_cast<uint>(CommendPoolType::MAX_ENUM));
	for (uint i = 0; i < static_cast<uint>(CommendPoolType::MAX_ENUM); i++)
		commendPools.emplace_back(*this, static_cast<CommendPoolType>(i));
}

LogicalDevice::LogicalDevice(
  const PhysicalDevice& p, QueueFamilyIndex queueFamilyIndex):
  pd(p) {
	queueIndex = queueFamilyIndex;
	_setup();
}
LogicalDevice::LogicalDevice(
  PhysicalDevice&& p, QueueFamilyIndex queueFamilyIndex):
  pd(std::move(p)) {
	queueIndex = queueFamilyIndex;
	_setup();
}
LogicalDevice::LogicalDevice(LogicalDevice&& o) noexcept:
  pd(std::move(o.pd)), commendPools(std::move(o.commendPools)) {
	queue = o.queue;
	queueIndex = o.queueIndex;
	d = o.d;
	o.d = nullptr;
}
LogicalDevice::~LogicalDevice() {
	if (swapChain) swapChain.reset();
	commendPools.clear();
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

bool LogicalDevice::isSwapchainValid() const {
	return swapChain && swapChain->isValid();
}


SwapChain& LogicalDevice::getSwapchain() { return *swapChain; }

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
CommendPool& LogicalDevice::getCommendPool(CommendPoolType type) {
	if constexpr (DO_SAFETY_CHECK)
		if (static_cast<uint>(type)
			>= static_cast<uint>(CommendPoolType::MAX_ENUM))
			throw "VulkanInvalidEnum";
	return commendPools.at(static_cast<uint>(type));
}
CommendBuffer LogicalDevice::getSingleUseCommendBuffer() {
	auto& cp = getCommendPool(CommendPoolType::Shortlived);
	return cp.makeCommendBuffer();
}

OSRenderSurface::OSRenderSurface() {
	if (glfwCreateWindowSurface(getVkInstance(),
		  (GLFWwindow*)events::ThreadEvents::getGLFWWindow(), nullptr, &surface)
		!= VK_SUCCESS) {
		logger("Vulkan and GLFW failed to get OS specific Render Surface!\n");
		throw "VulkanGLFWCreateOSWindowSurfaceFailure";
	}
}
OSRenderSurface::~OSRenderSurface() {
	vkDestroySurfaceKHR(getVkInstance(), surface, nullptr);
}

SwapChainSupport::SwapChainSupport(
  const PhysicalDevice& pd, const OSRenderSurface& s) {
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd.d, s.surface, &capabilities);
	uint fCount, pmCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(pd.d, s.surface, &fCount, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
	  pd.d, s.surface, &pmCount, nullptr);
	formats.resize(fCount);
	presentModes.resize(pmCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(
	  pd.d, s.surface, &fCount, formats.data());
	vkGetPhysicalDeviceSurfacePresentModesKHR(
	  pd.d, s.surface, &pmCount, presentModes.data());
}
VkSurfaceFormatKHR SwapChainSupport::getFormat() const {
	assert(!formats.empty());
	auto it = std::find_if(
	  formats.begin(), formats.end(), [](const VkSurfaceFormatKHR& v) {
		  return (v.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR
				  && v.format == VK_FORMAT_R8G8B8A8_SRGB);
	  });
	return it == formats.end() ? formats.front() : *it;
}
VkPresentModeKHR SwapChainSupport::getPresentMode(
  bool preferRelaxedVBlank) const {
#ifdef USE_MAILBOX_MODE
	return VK_PRESENT_MODE_MAILBOX_KHR;
#endif
	if (preferRelaxedVBlank
		&& std::find(presentModes.begin(), presentModes.end(),
			 VK_PRESENT_MODE_FIFO_RELAXED_KHR)
			 != presentModes.end())
		return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	else
		return VK_PRESENT_MODE_FIFO_KHR;
}
uvec2 SwapChainSupport::getSwapChainSize(uvec2 ps) const {
	if (capabilities.currentExtent.width != uint(-1)
		&& capabilities.currentExtent.height != uint(-1))
		return uvec2(
		  capabilities.currentExtent.width, capabilities.currentExtent.height);
	uvec2 min{
	  capabilities.minImageExtent.width, capabilities.minImageExtent.height};
	uvec2 max{
	  capabilities.maxImageExtent.width, capabilities.maxImageExtent.height};
	return glm::max(min, glm::min(max, ps));
}
uint SwapChainSupport::getImageCount(uint preferredCount) const {
	/*logger("ImgCount: ",
	  std::max(capabilities.minImageCount,
		capabilities.maxImageCount == 0
		  ? preferredCount
		  : std::min(capabilities.maxImageCount, preferredCount)),
	  " PreferredCount: ", preferredCount,
	  " Capabilities: ", capabilities.minImageCount, "-",
	  capabilities.maxImageCount);*/
	return std::max(capabilities.minImageCount,
	  capabilities.maxImageCount == 0
		? preferredCount
		: std::min(capabilities.maxImageCount, preferredCount));
}

static SwapchainCallback swapchainCallback;
void SwapChain::setCallback(SwapchainCallback sc) { swapchainCallback = sc; }

// NOTE: Three result from this _build() call:
// 1: return & Successful. sc != nullptr. *old_sc invalidated.
// 2: return & Failure. sc == nullptr. *old_sc invalidated.
// 3: throw Exception. *sc not touched. *old_sc not touched.
void SwapChain::_build(uvec2 preferredSize, uint preferredImageCount,
  WindowTransparentType transparentWindowType) {
	VkSwapchainKHR old_sc = sc;
	auto sp = d.pd.getSwapChainSupport();
	surfaceFormat = sp.getFormat();
	auto presentMode = sp.getPresentMode(true);
	pixelSize = sp.getSwapChainSize(preferredSize);
	if (pixelSize == uvec2(0)) {
		logger("Vulkan: Noted that window size is 0. Halting rendering...\n");
		if (swapchainCallback.onSurfaceMinimized)
			swapchainCallback.onSurfaceMinimized(*this);
		throw SurfaceMinimizedException();
	}

	uint32_t bufferCount = sp.getImageCount(preferredImageCount);

	if ((sp.capabilities.supportedCompositeAlpha
		  & (VkCompositeAlphaFlagBitsKHR)transparentWindowType)
		== 0) {
		logger("Vulkan: WARNING: Window Surface does not support "
			   "VkCompositeAlphaFlagBitsKHR: ",
		  (VkCompositeAlphaFlagBitsKHR)transparentWindowType,
		  ". Setting it to default.\n");
		transparentWindowType = WindowTransparentType::RemoveAlpha;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = sf.surface;
	createInfo.minImageCount = bufferCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = VkExtent2D{pixelSize.x, pixelSize.y};
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = sp.capabilities.currentTransform;
	createInfo.compositeAlpha =
	  (VkCompositeAlphaFlagBitsKHR)transparentWindowType;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = old_sc == nullptr ? VK_NULL_HANDLE : old_sc;
	auto result = vkCreateSwapchainKHR(d.d, &createInfo, nullptr, &sc);
	switch (result) {
	case VK_SUCCESS:
		vkGetSwapchainImagesKHR(d.d, sc, &bufferCount, nullptr);
		swapChainImages.resize(bufferCount);
		vkGetSwapchainImagesKHR(d.d, sc, &bufferCount, swapChainImages.data());
		outdated = false;
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		throw "VulkanSwapchainCreateFailure::OutOfHostMemory";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		throw "VulkanSwapchainCreateFailure::OutOfDeviceMemory";
	case VK_ERROR_DEVICE_LOST:
		// TODO: Add handling for device lost
		throw "TODOVulkanDeviceLostUnhandled";
	case VK_ERROR_SURFACE_LOST_KHR:
		// TODO: Add handling for surface lost
		throw "TODOVulkanSurfaceLostUnhandled";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		throw "VulkanSwapchainCreateFailure::NativeWindowInUse";
	case VK_ERROR_INITIALIZATION_FAILED: sc = nullptr;
	default: throw "VulkanSwapchainCreateFailure::UnhandledEnum";
	}
	if (old_sc != nullptr) {
		vkDestroySwapchainKHR(d.d, old_sc, nullptr);
		// TODO: Maybe seperate destruction of ticks and delay the
		// onDestroy callback?
		ticks.clear();
		if (swapchainCallback.onDestroy)
			swapchainCallback.onDestroy(*this, sc != nullptr);
	}
	if (sc != nullptr) {
		ticks.resize(getImageCount());
		if (swapchainCallback.onCreate)
			swapchainCallback.onCreate(*this, old_sc != nullptr);
	}
}

SwapChain::SwapChain(const OSRenderSurface& surface, LogicalDevice& device,
  uvec2 preferredSize, uint preferredImageCount,
  WindowTransparentType transparentWindowType):
  d(device),
  sf(surface) {
	// logger("Create SwapChain...");
	sc = nullptr;
	_build(preferredSize, preferredImageCount, transparentWindowType);
}
void SwapChain::rebuild(uvec2 preferredSize, uint preferredImageCount,
  WindowTransparentType transparentWindowType) {
	// logger("Rebuild SwapChain...");
	_build(preferredSize, preferredImageCount, transparentWindowType);
}
uint SwapChain::getImageCount() const { return swapChainImages.size(); }
bool SwapChain::renderNextFrame(bool waitForVSync) {
	uint imageId{uint(-1)};
	Semaphore sp{d};
	auto code =
	  vkAcquireNextImageKHR(d.d, sc, 0, sp.sp, VK_NULL_HANDLE, &imageId);
	switch (code) {
	case VK_SUBOPTIMAL_KHR: outdated = true;
	case VK_SUCCESS: {
		auto& optPtr = ticks[imageId];
		// logger("Swapchain Acquire ", imageId);
		if (optPtr) {
			// logger("Swapchain reset Tick ", imageId);
			optPtr->reset(std::move(sp));
		} else {
			// logger("Swapchain Make Tick ", imageId);
			optPtr.make(d, imageId, std::move(sp));
		}
		while (!optPtr->render()) {
			// FIXME: Kinda hacky here using std::move on another obj's internal
			// stuff Currently needs a placeholder to avoid self move asignment
			// issue causing underfined(?) behavior or unspecified state.
			Semaphore temp{std::move(optPtr->acquireSemaphore)};
			optPtr.make(d, imageId, std::move(temp));
		}
		try {
			optPtr->send();	 // May throw OutdatedSwapchainException()
			return true;
		} catch (OutdatedSwapchainException) {
			outdated = true;
			return false;
		}
	}
	case VK_ERROR_OUT_OF_DATE_KHR: outdated = true; return false;
	case VK_TIMEOUT: return false;
	case VK_NOT_READY: return false;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		throw "VulkanSwapchainGetNextImageFailure::OutOfHostMemory";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		throw "VulkanSwapchainGetNextImageFailure::OutOfDeviceMemory";
	case VK_ERROR_DEVICE_LOST:
		// TODO: Add handling for device lost
		throw "TODOVulkanDeviceLostUnhandled";
	case VK_ERROR_SURFACE_LOST_KHR:
		// TODO: Add handling for surface lost
		throw "TODOVulkanSurfaceLostUnhandled";
	default: throw "VulkanSwapchainGetNextImageFailure::UnhandledEnum";
	}
}

bool SwapChain::isValid() const { return sc != nullptr && !outdated; }

ImageView SwapChain::getChainImageView(uint index) {
	VkImageViewCreateInfo cInfo{};
	cInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	cInfo.image = swapChainImages.at(index);
	cInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	cInfo.format = surfaceFormat.format;
	cInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cInfo.subresourceRange.baseMipLevel = 0;
	cInfo.subresourceRange.levelCount = 1;
	cInfo.subresourceRange.baseArrayLayer = 0;
	cInfo.subresourceRange.layerCount = 1;
	return ImageView(d, cInfo);
}
SwapChain::~SwapChain() {
	if (sc != nullptr) {
		ticks.clear();
		if (swapchainCallback.onDestroy)
			swapchainCallback.onDestroy(*this, false);
		vkDestroySwapchainKHR(d.d, sc, nullptr);
	}
}