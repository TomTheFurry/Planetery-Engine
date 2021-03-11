#include "VK.h"

#include "Logger.h"

#include <algorithm>
#include <glfw/glfw3.h>

VkResult vkCreateDebugUtilsMessengerEXT(VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
  const VkAllocationCallbacks* pAllocator,
  VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
	  instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
void vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
	  instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) { func(instance, debugMessenger, pAllocator); }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
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



using namespace vk;

static bool _loadedPreInitData = false;
static std::vector<VkLayerProperties> _layersAvailable;
static std::vector<VkExtensionProperties> _extensionsAvailable;
static std::vector<VkPhysicalDevice> _physicalDevicesAvailable;
static std::vector<VkPhysicalDevice> _physicalDevices;
static std::vector<const char*> _layersEnabled;
static std::vector<const char*> _extensionsEnabled;
static VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
static VkInstance _vk = nullptr;

constexpr const std::pair<const char*, uint> LAYERS[] = {
  {"VK_LAYER_KHRONOS_validation", 0u},
};

constexpr const std::pair<const char*, uint> EXTENSIONS[] = {
  {"VK_EXT_debug_utils", 0u},
  {"VK_EXT_validation_features", 0u},
  {"VK_EXT_debug_report", 0u},
};

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
inline void sortPhysicalDevices() {
	logger.newLayer();
	logger << "Vulkan: Scanning Graphic Cards...\n";
	std::vector<std::pair<int, VkPhysicalDevice>> _preSort;
	_preSort.reserve(_physicalDevicesAvailable.size());
	for (auto& d : _physicalDevicesAvailable) {
		logger.newLayer();
		VkPhysicalDeviceProperties dPop;
		VkPhysicalDeviceFeatures dFea;
		vkGetPhysicalDeviceProperties(d, &dPop);
		vkGetPhysicalDeviceFeatures(d, &dFea);
		auto& dLim = dPop.limits;
		logger << dPop.deviceName << "...";
		// GPU Requirements
		if (!dFea.geometryShader) continue;
		if (!dFea.tessellationShader) continue;
		logger << " Usable. Rating: ";
		int rating = 0;
		// GPU Ratings
		if (dPop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			rating += 1000;
		if (dPop.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			rating += 100;
		rating += std::log(double(dLim.maxComputeWorkGroupSize[0])
						   * double(dLim.maxComputeWorkGroupSize[1])
						   * double(dLim.maxComputeWorkGroupSize[2]))*100;
		rating += dLim.maxGeometryOutputVertices;
		rating += dLim.maxGeometryTotalOutputComponents;
		rating += std::log(double(dLim.sparseAddressSpaceSize))*100;
		_preSort.emplace_back(rating, d);

		logger << std::to_string(rating) << "\n";
		logger.closeLayer();
	}
	_physicalDevices.reserve(_preSort.size());
	std::sort(_preSort.begin(), _preSort.end(),
	  [](const auto& p1, const auto& p2) { return p1.first < p2.first; });
	for (auto& p : _preSort) _physicalDevices.push_back(p.second);
	logger.closeLayer();
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

void vk::init() {
	logger("VK Interface init.\n");
	// Checkin the default requested extensions and layers
	{
		bool noMissingLayer = true;
		for (const auto& p : LAYERS) {
			if (p.first == "") continue;
			if (!requestLayer(p.first, p.second)) {
				logger("Vulkan: Missing default requested layer: ", p.first,
				  "(minV:", p.second, ")\n");
				noMissingLayer = false;
			}
		}
		if (!noMissingLayer) throw "VulkanMissingExtension";
	};
	{
		bool noMissingExtension = true;
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (; glfwExtensionCount > 0; glfwExtensionCount--) {
			if (!requestExtension(
				  *(glfwExtensions + (glfwExtensionCount - 1)))) {
				logger("Vulkan: Missing glfw requested extension: ",
				  *(glfwExtensions + (glfwExtensionCount - 1)), "\n");
				noMissingExtension = false;
			}
		}
		for (const auto& p : EXTENSIONS) {
			if (p.first == "") continue;
			if (!requestExtension(p.first, p.second)) {
				logger("Vulkan: Missing default requested extension: ", p.first,
				  "(minV:", p.second, ")\n");
				noMissingExtension = false;
			}
		}
		if (!noMissingExtension) throw "VulkanMissingExtension";
	};
	// Create the Vulkan instance
	{
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
		createInfo.enabledLayerCount = _layersEnabled.size();
		createInfo.ppEnabledLayerNames = _layersEnabled.data();
		createInfo.enabledExtensionCount = _extensionsEnabled.size();
		createInfo.ppEnabledExtensionNames = _extensionsEnabled.data();
		if (vkCreateInstance(&createInfo, nullptr, &_vk) != VK_SUCCESS) {
			logger("Failed to initialize Vulkan!\n");
			throw "VulkanCreateInstanceFailure";
		}
	}
	// Create the Vulkan debug logger
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType =
		  VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
		  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		  | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
		  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
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
	// Create the gpu handle
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(_vk, &deviceCount, nullptr);
		if (deviceCount == 0) {
			logger("Vulkan did not find any available graphic cards!\n");
			throw "VulkanNoGraphicCardsDetected";
		}
		_physicalDevicesAvailable.resize(deviceCount);
		vkEnumeratePhysicalDevices(
		  _vk, &deviceCount, _physicalDevicesAvailable.data());
		sortPhysicalDevices();
		if (_physicalDevices.empty()) {
			logger("Vulkan did not find any usable graphic cards!\n");
			throw "VulkanNoUsableGraphicCardsDetected";
		}
	}
}

void vk::end() {
	logger("VK Interface end.\n");
	if (_debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(_vk, _debugMessenger, nullptr);
	if (_vk) vkDestroyInstance(_vk, nullptr);
}
