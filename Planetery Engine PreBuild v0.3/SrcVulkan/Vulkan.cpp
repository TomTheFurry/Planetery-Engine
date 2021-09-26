module;
#include "Marco.h"
//#include <cstdlib>
module Vulkan;
import: Device;
import: Buffer;
import: Image;
import: Shader;
import: Sync;
import: Commend;
import: Descriptor;
import: Pipeline;
import: Enum;
import: Swapchain;
import std.core;
import Define;
import Logger;
// import "Assert.h";
import "VulkanExtModule.h";
import "GlfwModule.h";
using namespace vk;

static VkInstance _vk = nullptr;
#ifdef VULKAN_DEBUG
#	pragma warning(suppress : 26812)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		return VK_FALSE;
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		return VK_FALSE;
	if (pCallbackData->messageIdNumber == 2094043421) return VK_FALSE;
	logger.newMessage();
	logger << "VulkanDebugCallback:\n";
	// size_t len = std::strlen(pCallbackData->pMessage);
	// char* c = new char[len];
	// const char* base = pCallbackData->pMessage;
	// for (size_t p = 0; *(base + p) != '\0'; p++) {
	//	*(base + p) == '|' ? *(c + p) = '\n' : *(c + p) = *(base + p);
	//}
	logger(pCallbackData->pMessage, "\n");
	// delete[] c;
	logger.closeMessage();
	return VK_FALSE;
}
static VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
inline void createDebugger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
							   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
#	pragma warning(suppress : 26812)
	if (vkCreateDebugUtilsMessengerEXT(
		  _vk, &createInfo, nullptr, &_debugMessenger)
		!= VK_SUCCESS) {
		logger("Warning: Failed to create Vulkan logger!\n");
	};
}
#endif

// Const Settings
const Layer _layers[] = {
#ifdef VULKAN_DEBUG
  {"VK_LAYER_KHRONOS_validation", 0u},
#endif
  {"", 0u},
};
const Extension _extensions[] = {
#ifdef VULKAN_DEBUG
  {"VK_EXT_debug_utils", 0u},
  {"VK_EXT_validation_features", 0u},
//{"VK_EXT_debug_report", 0u},
#endif
  {"", 0u},
};
const char* const _deviceExtensions[] = {
  "VK_KHR_swapchain",
};

// Static values
static uint32_t glfwExtensionCount;
static bool _loadedPreInitData = false;
static std::vector<const char*> _layersEnabled;
static std::vector<const char*> _extensionsEnabled;
static std::vector<VkLayerProperties> _layersAvailable;
static std::vector<VkExtensionProperties> _extensionsAvailable;
static DeviceCallback dCallback;
static OSRenderSurface* _OSSurface = nullptr;
static PhysicalDevice* _physicalDevice = nullptr;
static LogicalDevice* _renderDevice = nullptr;
//unused static uint _currentFrame = 0;
static bool _newSwapchain = true;
static std::atomic_flag _swapchainNotOutdated;

inline void preInit() {
	_loadedPreInitData = true;
	logger("Vulkan: Reading runtime values...\n");
	{
		uint count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		_layersAvailable = std::vector<VkLayerProperties>(count);
		vkEnumerateInstanceLayerProperties(&count, _layersAvailable.data());
	}
	if constexpr (IS_DEBUG_MODE) {
		logger.newLayer();
		logger << "Avalible VK Layers: \n";
		for (auto& l : _layersAvailable) {
			logger(
			  l.specVersion, ": ", l.layerName, " - ", l.description, "\n");
		}
		logger.closeLayer();
	}
	{
		uint count;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		_extensionsAvailable.resize(count);
		vkEnumerateInstanceExtensionProperties(
		  nullptr, &count, _extensionsAvailable.data());
	}
	for (auto& layer : _layersAvailable) {
		uint count;
		vkEnumerateInstanceExtensionProperties(
		  layer.layerName, &count, nullptr);
		_extensionsAvailable.resize(_extensionsAvailable.size() + count);
		vkEnumerateInstanceExtensionProperties(layer.layerName, &count,
		  _extensionsAvailable.data() + _extensionsAvailable.size() - count);
	}
	if constexpr (IS_DEBUG_MODE) {
		logger.newLayer();
		logger << "Avalible VK Extensions: \n";
		for (auto& ex : _extensionsAvailable) {
			logger(ex.specVersion, ":", ex.extensionName, "\n");
		}
		logger.closeLayer();
	}
}

bool vk::requestLayer(const char* name, uint minVersion) {
	if (!_loadedPreInitData) preInit();
	auto it = std::find_if(_layersAvailable.begin(), _layersAvailable.end(),
	  [name, minVersion](const VkLayerProperties& lay) {
		  return (
			strcmp(name, lay.layerName) == 0 && minVersion <= lay.specVersion);
	  });
	if (it == _layersAvailable.end()) return false;
	_layersEnabled.push_back(it->layerName);
	return true;
}
bool vk::requestExtension(const char* name, uint minVersion) {
	if (!_loadedPreInitData) preInit();
	auto it =
	  std::find_if(_extensionsAvailable.begin(), _extensionsAvailable.end(),
		[name, minVersion](const VkExtensionProperties& ext) {
			return (strcmp(name, ext.extensionName) == 0
					&& minVersion <= ext.specVersion);
		});
	if (it == _extensionsAvailable.end()) return false;
	_extensionsEnabled.push_back(it->extensionName);
	return true;
}

std::span<const Layer> vk::getRequestedLayers() {
	return std::span<const Layer>(_layers);
}
std::span<const Extension> vk::getRequestedExtensions() {
	return std::span<const Extension>(_extensions);
}
std::span<const char* const> vk::getRequestedDeviceExtensions() {
	return std::span<const char* const>(_deviceExtensions);
}

VkInstance vk::getVkInstance() { return _vk; }

inline void checkBaseRequiredPlugins() {
	bool noMissingLayer = true;
	for (const auto& l : getRequestedLayers()) {
		if (l.name[0] == '\0') continue;
		if (!requestLayer(l.name, l.version)) {
			logger("Vulkan: Missing default requested layer: ", l.name,
			  "(minV:", l.version, ")\n");
			noMissingLayer = false;
		}
	}
	if (!noMissingLayer) throw "VulkanMissingExtension";

	bool noMissingExtension = true;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	uint glfwc = glfwExtensionCount;
	for (; glfwc > 0; glfwc--) {
		if (!requestExtension(*(glfwExtensions + (glfwc - 1)))) {
			logger("Vulkan: Missing glfw requested extension: ",
			  *(glfwExtensions + (glfwExtensionCount - 1)), "\n");
			noMissingExtension = false;
		}
	}
	for (const auto& e : getRequestedExtensions()) {
		if (e.name[0] == '\0') continue;
		if (!requestExtension(e.name, e.version)) {
			logger("Vulkan: Missing default requested extension: ", e.name,
			  "(minV:", e.version, ")\n");
			noMissingExtension = false;
		}
	}
	if (!noMissingExtension) throw "VulkanMissingExtension";
}
inline void createInstance() {
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Planetery Game";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.pEngineName = "Planetery Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_2;
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = (uint)_layersEnabled.size();
	createInfo.ppEnabledLayerNames = _layersEnabled.data();
	createInfo.enabledExtensionCount = (uint)_extensionsEnabled.size();
	createInfo.ppEnabledExtensionNames = _extensionsEnabled.data();
	if (vkCreateInstance(&createInfo, nullptr, &_vk) != VK_SUCCESS) {
		logger("Failed to initialize Vulkan!\n");
		throw "VulkanCreateInstanceFailure";
	}
}

void vk::init() {
	logger("VK Interface init.\n");
	// Checkin the default requested extensions and layers
	checkBaseRequiredPlugins();
	// Create the Vulkan instance
	createInstance();
#ifdef VULKAN_DEBUG
	// Create the Vulkan debug logger
	createDebugger();
#endif
	// Create the OS specific Render Surface (for display out)
	_OSSurface = new OSRenderSurface();
	// Create physical device
	_physicalDevice = PhysicalDevice::getUsablePhysicalDevice(_OSSurface);
	// Calculate queue layout for this computer
	QueuePoolLayout qpl{*_physicalDevice};
	uint rQueueFamily = qpl.findFamilyBySupportType(QueueType::Graphics);
	if constexpr (DO_SAFETY_CHECK)
		if (rQueueFamily == uint(-1))
			throw "VulkanInternalErrorPhysicalDeviceHasNoRenderQueue";
	if (!qpl.checkQueuePresentSupport(rQueueFamily, *_OSSurface)) {
		// TODO: Make this find another queue that support presenting on target
		// surface
		throw "TODOVulkanMainRenderQueueDoesNotSupportPresentOnTargetOSSurface";
	}
	uint mQueueFamily = qpl.findFamilyBySupportType(
	  QueueType::MemoryTransfer, Flags(QueueType::Graphics) | QueueType::Compute);
	//logger(mQueueFamily);
	if (mQueueFamily == uint(-1))
		mQueueFamily = qpl.findFamilyBySupportType(
		  QueueType::MemoryTransfer, QueueType::Graphics);
	//logger(mQueueFamily);
	if (mQueueFamily == uint(-1) || mQueueFamily==rQueueFamily) {
		// Use render family for memory transfer
		uint rQueueCount =
		  std::min(qpl.getFamilyMaxQueueCount(rQueueFamily), 12u);
		qpl.addQueueGroup(rQueueFamily, rQueueCount);
		qpl.hintGroupUsage(Memory, rQueueFamily);
		qpl.hintGroupUsage(Present, rQueueFamily);
		qpl.hintGroupUsage(Render, rQueueFamily);
	} else {
		uint rQueueCount =
		  std::min(qpl.getFamilyMaxQueueCount(rQueueFamily), 8u);
		uint mQueueCount = std::min(qpl.getFamilyMaxQueueCount(mQueueFamily), 4u);
		qpl.addQueueGroup(rQueueFamily, rQueueCount);
		qpl.addQueueGroup(mQueueFamily, mQueueCount);
		qpl.hintGroupUsage(Memory, mQueueFamily);
		qpl.hintGroupUsage(Present, rQueueFamily);
		qpl.hintGroupUsage(Render, rQueueFamily);
	}
	_renderDevice = new LogicalDevice(*_physicalDevice, qpl);
	dCallback.onCreate(*_renderDevice);

	//Swapchain is lazily created when drawing via _OSSurface
}
bool vk::drawFrame(ulint timeout) {
	try {
		Swapchain* sc = &_OSSurface->querySwapchain(*_renderDevice);
		if (sc == nullptr) return false;
		sc->renderNextImage(timeout);
		return true;
	} catch (SurfaceMinimizedException) { return false; }
	//unused _currentFrame++;
}
void vk::end() {
	logger("VK Interface end.\n");
	if (_OSSurface != nullptr) _OSSurface->releaseSwapchain();
	if (_renderDevice != nullptr) dCallback.onDestroy(*_renderDevice);
	delete _renderDevice;
	delete _OSSurface;
#ifdef VULKAN_DEBUG
	if (_debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(_vk, _debugMessenger, nullptr);
#endif
	if (_vk) vkDestroyInstance(_vk, nullptr);
}
void vk::setCallback(DeviceCallback dC) { dCallback = dC; }
void vk::setCallback(SwapchainCallback scC) { Swapchain::setCallback(scC); }
void vk::setCallback(FrameCallback fC) { Swapchain::setCallback(fC); }
