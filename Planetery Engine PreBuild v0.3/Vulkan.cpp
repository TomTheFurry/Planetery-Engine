module;
#include "Marco.h"
#ifdef USE_VULKAN
#	pragma warning(disable : 26812)
#	include <vulkan/vulkan.h>
#	include "GLFW.h"
#	include <assert.h>
module Vulkan;
import: Internal;
import: Enum;
import std.core;
import Define;
import Logger;
using namespace vk;

VkResult __cdecl vkCreateDebugUtilsMessengerEXT(VkInstance instance,
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
void __cdecl vkDestroyDebugUtilsMessengerEXT(VkInstance instance,
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

const Layer _layers[] = {
  {"VK_LAYER_KHRONOS_validation", 0u},
};
const Extension _extensions[] = {
  {"VK_EXT_debug_utils", 0u}, {"VK_EXT_validation_features", 0u},
  //{"VK_EXT_debug_report", 0u},
};
const char* const _deviceExtensions[] = {
  "VK_KHR_swapchain",
};
static VkInstance _vk = nullptr;
static bool _loadedPreInitData = false;
static std::vector<VkLayerProperties> _layersAvailable;
static std::vector<VkExtensionProperties> _extensionsAvailable;
static std::list<PhysicalDevice> _physicalDevices;
static OSRenderSurface* _OSSurface;
static std::vector<const char*> _layersEnabled;
static std::vector<const char*> _extensionsEnabled;
static VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
static LogicalDevice* _renderDevice = nullptr;
static uint32_t glfwExtensionCount;


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
inline void scanPhysicalDevices() {
	std::vector<VkPhysicalDevice> _physicalDevicesAvailable;
	logger.newLayer();
	logger << "Vulkan: Scanning Graphic Cards...\n";
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(_vk, &deviceCount, nullptr);
		if (deviceCount == 0) {
			logger("Vulkan did not find any available graphic cards!\n");
			logger.closeLayer();
			throw "VulkanNoGraphicCardsDetected";
		}
		_physicalDevicesAvailable.resize(deviceCount);
		vkEnumeratePhysicalDevices(
		  _vk, &deviceCount, _physicalDevicesAvailable.data());
	}
	//_physicalDevices.reserve(_physicalDevicesAvailable.size());
	for (auto& d : _physicalDevicesAvailable) {
		PhysicalDevice dev(d, _OSSurface);
		if (dev.meetRequirements) _physicalDevices.emplace_back(std::move(dev));
	}
	_physicalDevices.sort(
	  [](const auto& p1, const auto& p2) { return p1 < p2; });
	if (_physicalDevices.empty()) {
		logger("Vulkan did not find any usable graphic cards!\n");
		logger.closeLayer();
		throw "VulkanNoUsableGraphicCardsDetected";
	}
	logger.closeLayer();
}
inline void makeRenderingLogicalDevice() {
	_renderDevice = _physicalDevices.front().makeDevice(VK_QUEUE_GRAPHICS_BIT);
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

static RenderPass* _renderPass = nullptr;
static ShaderPipeline* _pipeline = nullptr;
static VertexBuffer* _vertBuff = nullptr;
static IndexBuffer* _indexBuff = nullptr;
static std::vector<ImageView> _swapchainViews{};
static std::vector<FrameBuffer> _frameBuffers{};
static VkSemaphore _imageAvailableSemaphore;
static VkSemaphore _renderFinishedSemaphore;
static std::vector<CommendBuffer> _commendBuffers{};
constexpr auto MAX_FRAMES_IN_FLIGHT = 2;
static std::array<RenderTick*, MAX_FRAMES_IN_FLIGHT> _renderFrames{nullptr};
static uint _currentFrame = 0;
static bool _newSwapchain = true;
static std::atomic_flag _swapchainNotOutdated;

const float testVert[]{
  0.5f,
  0.5f,
  -0.5f,
  0.5f,
  0.5f,
  -0.5f,
  -0.5f,
  -0.5f,
};
const float testVert2[]{
  0.2f,
  0.2f,
  -0.2f,
  0.2f,
  0.2f,
  -0.2f,
  -0.2f,
  -0.2f,
};

const uint testInd[]{
  0,
  1,
  2,
  3,
};

inline void loadSwapchain(bool remake = false) {
	_newSwapchain = true;
	_swapchainNotOutdated.test_and_set(std::memory_order_relaxed);
	if (remake) _renderDevice->remakeSwapChain();
	else
		_renderDevice->makeSwapChain();
	// Make renderPass
	_renderPass = new RenderPass(*_renderDevice);
	auto& ad = _renderPass->attachmentTypes.emplace_back();
	ad.format = _renderDevice->swapChain->surfaceFormat.format;
	ad.samples = VK_SAMPLE_COUNT_1_BIT;
	ad.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ad.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ad.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ad.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ad.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ad.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	_renderPass->subpasses.emplace_back(1, 0);
	auto& sd = _renderPass->subpassDependencies.emplace_back();
	sd.srcSubpass = VK_SUBPASS_EXTERNAL;
	sd.dstSubpass = 0;
	sd.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sd.srcAccessMask = 0;
	sd.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	sd.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	_renderPass->complete();
	ShaderCompiled vertShad(
	  *_renderDevice, ShaderType::Vert, "cshader/point.cvert");
	ShaderCompiled fragShad(
	  *_renderDevice, ShaderType::Frag, "cshader/point.cfrag");
	// Make Pipeline
	_pipeline = new ShaderPipeline(*_renderDevice);
	std::vector<const ShaderCompiled*> pointShad;
	pointShad.reserve(2);
	pointShad.push_back(&vertShad);
	pointShad.push_back(&fragShad);
	VkViewport viewport{
	  .x = 0,
	  .y = 0,
	  .width = (float)_renderDevice->swapChain->pixelSize.x,
	  .height = (float)_renderDevice->swapChain->pixelSize.y,
	  .minDepth = 0,
	  .maxDepth = 0,
	};
	VertexAttribute va{};
	va.addAttributeByType<vec2>();
	va.addBindingPoint();
	_pipeline->complete(pointShad, va, viewport, *_renderPass);
	assert(_pipeline->p != nullptr);

	// Make swapchain framebuffer
	_swapchainViews.reserve(_renderDevice->swapChain->swapChainImages.size());
	_frameBuffers.reserve(_renderDevice->swapChain->swapChainImages.size());
	for (uint i = 0; i < _renderDevice->swapChain->swapChainImages.size();
		 i++) {
		_swapchainViews.push_back(
		  _renderDevice->swapChain->getChainImageView(i));
		std::vector<ImageView*> b;
		b.emplace_back(&_swapchainViews.back());
		_frameBuffers.emplace_back(
		  *_renderDevice, *_renderPass, _renderDevice->swapChain->pixelSize, b);
	}
}
inline void unloadSwapchain() {
	for (auto& ptr : _renderFrames)
		if (ptr != nullptr) {
			delete ptr;
			ptr = nullptr;
		}
	_frameBuffers.clear();
	_swapchainViews.clear();
	if (_pipeline != nullptr) delete _pipeline;
	if (_renderPass != nullptr) delete _renderPass;
}
inline void recreateSwapchain() {
	unloadSwapchain();
	loadSwapchain(true);
}

void vk::init() {
	logger("VK Interface init.\n");
	// Checkin the default requested extensions and layers
	checkBaseRequiredPlugins();
	// Create the Vulkan instance
	createInstance();
	// Create the Vulkan debug logger
	createDebugger();
	// Create the OS specific Render Surface (for display out)
	_OSSurface = new OSRenderSurface();
	// Create devices
	scanPhysicalDevices();
	makeRenderingLogicalDevice();
	// Make SwapChain and pipeline
	loadSwapchain();
	// Test programe
	// Make vertex buffer
	_vertBuff = new VertexBuffer(
	  *_renderDevice, sizeof(testVert), MemoryFeature::IndirectWritable);
	_vertBuff->blockingIndirectWrite((void*)std::data(testVert));
	_indexBuff = new IndexBuffer(
	  *_renderDevice, sizeof(testInd), MemoryFeature::IndirectWritable);
	_indexBuff->blockingIndirectWrite((void*)std::data(testInd));
}

static RenderTick* _currentRenderTick = nullptr;
void vk::_prepareFrame() {
	auto& frame = _renderFrames[_currentFrame];
	if (frame != nullptr) {
		delete frame;
		frame = nullptr;
	}
	checkStatus();
	frame = new RenderTick(*_renderDevice);
	_currentRenderTick = frame;
}
void vk::_sendFrame() {
	checkStatus();
	_currentRenderTick->send();
	_currentFrame++;
	_currentFrame %= MAX_FRAMES_IN_FLIGHT;
	_newSwapchain = false;
}
void vk::_resetOutdatedFrame() { recreateSwapchain(); }

void vk::_testDraw() {
	if (_newSwapchain) {
		_commendBuffers.clear();
		_commendBuffers.reserve(
		  _renderDevice->swapChain->swapChainImages.size());
		for (uint i = 0; i < _renderDevice->swapChain->swapChainImages.size();
			 i++) {
			auto& cb = _commendBuffers.emplace_back(
			  _renderDevice->getCommendPool(CommendPoolType::Default));
			cb.startRecording(CommendBufferUsage::None);
			cb.cmdBeginRender(
			  *_renderPass, _frameBuffers.at(i), vec4(1., 0., 0., 0.));
			cb.cmdBind(*_pipeline);
			cb.cmdBind(*_vertBuff);
			cb.cmdBind(*_indexBuff);
			// cb.cmdDraw((uint)std::size(testVert) / 2);
			cb.cmdDrawIndexed(std::size(testInd));
			cb.cmdEndRender();
			cb.endRecording();
		}
	}
	_currentRenderTick->addCmdStage(
	  _commendBuffers.at(_currentRenderTick->getImageIndex()), {}, {},
	  {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT});
}

void vk::checkStatus() noexcept(false) {
	if (!_swapchainNotOutdated.test_and_set(std::memory_order_relaxed))
		throw OutdatedSwapchainException{};
}

void vk::end(void (*cleanupFunc)()) {
	logger("VK Interface end.\n");
	unloadSwapchain();
	cleanupFunc();
	// testPrograme
	_commendBuffers.clear();
	if (_vertBuff != nullptr) delete _vertBuff;
	if (_indexBuff != nullptr) delete _indexBuff;
	// testPrograme end
	_physicalDevices.clear();
	if (_OSSurface != nullptr) delete _OSSurface;
	if (_debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(_vk, _debugMessenger, nullptr);
	if (_vk) vkDestroyInstance(_vk, nullptr);
}

void vk::notifyOutdatedSwapchain() {
	_swapchainNotOutdated.clear(std::memory_order_relaxed);
}

static bool useA = true;
void vk::testSwitch() {
	if (useA) {
		_vertBuff->blockingIndirectWrite((void*)std::data(testVert2));
	} else {
		_vertBuff->blockingIndirectWrite((void*)std::data(testVert));
	}
	useA = !useA;
}

#else
module Vulkan;
#endif