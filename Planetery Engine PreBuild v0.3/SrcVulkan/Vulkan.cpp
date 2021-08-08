module;
#include "Marco.h"
#include <cstdlib>
module Vulkan;
import: Device;
import: Buffer;
import: Image;
import: Shader;
import: Sync;
import: Commend;
import: Descriptor;
import: Pipeline;
import: Tick;
import: Enum;
import std.core;
import Define;
import Logger;
import "VulkanExtModule.h";
import "Assert.h";
import "GlfwModule.h";
using namespace vk;

static VkInstance _vk = nullptr;
#ifdef VULKAN_DEBUG
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
static SwapchainCallback scCallback;
static FrameCallback fCallback;
static OSRenderSurface* _OSSurface;
static LogicalDevice* _renderDevice = nullptr;
constexpr auto MAX_FRAMES_IN_FLIGHT = 4;
static std::array<RenderTick*, MAX_FRAMES_IN_FLIGHT> _renderFrames{nullptr};
static uint _currentFrame = 0;
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
		if (l.name == "") continue;
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
		if (e.name == "") continue;
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
inline void loadSwapchain(bool remake = false) {
	_newSwapchain = true;
	_swapchainNotOutdated.test_and_set(std::memory_order_relaxed);
	if (remake) _renderDevice->remakeSwapChain();
	else
		_renderDevice->makeSwapChain();

	if (scCallback.onCreate != nullptr)
		scCallback.onCreate(*_renderDevice->swapChain, remake);
}
inline void unloadSwapchain(bool remake = false) {
	for (auto& ptr : _renderFrames)
		if (ptr != nullptr) {
			delete ptr;
			ptr = nullptr;
		}
	if (scCallback.onDestroy != nullptr) scCallback.onDestroy(remake);
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
	// Create devices
	_renderDevice = PhysicalDevice::getUsablePhysicalDevice(_OSSurface)
					  .makeDevice(VK_QUEUE_GRAPHICS_BIT);
	dCallback.onCreate(*_renderDevice);
	// Make SwapChain and pipeline and programs and layouts
	if (scCallback.onCreate == nullptr && scCallback.onDestroy == nullptr) {
		logger("Warning! SwapchainCallback not set!\n");
	}
	loadSwapchain();
}
bool vk::drawFrame() {
	try {
		auto& frame = _renderFrames[_currentFrame];
		if (frame != nullptr) {
			delete frame;
			frame = nullptr;
		}
		frame = new RenderTick(*_renderDevice);
		fCallback.onDraw(*frame);
		frame->send();
		_currentFrame++;
		_currentFrame %= MAX_FRAMES_IN_FLIGHT;
		_newSwapchain = false;
		return true;
	} catch (OutdatedSwapchainException) {
		unloadSwapchain(true);
		loadSwapchain(true);
		return false;
	}
}
void vk::checkStatus() noexcept(false) {
	if (!_swapchainNotOutdated.test_and_set(std::memory_order_relaxed))
		throw OutdatedSwapchainException{};
}
void vk::end() {
	logger("VK Interface end.\n");
	unloadSwapchain();
	dCallback.onDestroy(*_renderDevice);
	if (_renderDevice != nullptr) delete _renderDevice;
	// testPrograme end
	if (_OSSurface != nullptr) delete _OSSurface;
#ifdef VULKAN_DEBUG
	if (_debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(_vk, _debugMessenger, nullptr);
#endif
	if (_vk) vkDestroyInstance(_vk, nullptr);
}
void vk::setCallback(DeviceCallback dC) { dCallback = dC; }
void vk::setCallback(SwapchainCallback scC) { scCallback = scC; }
void vk::setCallback(FrameCallback fC) { fCallback = fC; }

void vk::notifyOutdatedSwapchain() {
	_swapchainNotOutdated.clear(std::memory_order_relaxed);
}

