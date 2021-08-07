module;
#include "Marco.h"
#include <cstdlib>
// Image loading using stb. Remove this when not testing!!!
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
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
static VkInstance _vk = nullptr;
static bool _loadedPreInitData = false;
static std::vector<VkLayerProperties> _layersAvailable;
static std::vector<VkExtensionProperties> _extensionsAvailable;
static OSRenderSurface* _OSSurface;
static std::vector<const char*> _layersEnabled;
static std::vector<const char*> _extensionsEnabled;
#ifdef VULKAN_DEBUG
static VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
#endif
static LogicalDevice* _renderDevice = nullptr;
static uint32_t glfwExtensionCount;
static SwapchainCallback scCallback;


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
#ifdef VULKAN_DEBUG
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
  1.f,
  1.f,

  -0.5f,
  0.5f,
  0.f,
  1.f,

  0.5f,
  -0.5f,
  1.f,
  0.f,

  -0.5f,
  -0.5f,
  0.f,
  0.f,
};
const float testVert2[]{
  0.2f,
  0.2f,
  1.f,
  1.f,

  -0.2f,
  0.2f,
  0.f,
  1.f,

  0.2f,
  -0.2f,
  1.f,
  0.f,

  -0.2f,
  -0.2f,
  0.f,
  0.f,
};

const uint testInd[]{
  0,
  1,
  2,
  3,
};

static DescriptorLayout* _dsl = nullptr;
static ShaderCompiled* _vertShad = nullptr;
static ShaderCompiled* _fragShad = nullptr;

inline void loadSwapchain(bool remake = false) {
	_newSwapchain = true;
	_swapchainNotOutdated.test_and_set(std::memory_order_relaxed);
	if (remake) _renderDevice->remakeSwapChain();
	else
		_renderDevice->makeSwapChain();
	// Make renderPass
	VkFormat swapchainFormat = _renderDevice->swapChain->surfaceFormat.format;

	{
		RenderPass::Attachment swapchainAtm(swapchainFormat,
		  TextureActiveUseType::Undefined, AttachmentReadOp::Clear,
		  TextureActiveUseType::Present, AttachmentWriteOp::Write);
		RenderPass::SubPass subPass1({}, {}, {0}, {}, {}, {});
		RenderPass::SubPassDependency dependency1(uint(-1),
		  PipelineStage::OutputAttachmentColor,
		  MemoryAccess::None, 0,
		  PipelineStage::OutputAttachmentColor, MemoryAccess::AttachmentColorWrite,
			false);
		_renderPass = new RenderPass(
		  *_renderDevice, {swapchainAtm}, {subPass1}, {dependency1});
	}
	if (_vertShad == nullptr) {
		_vertShad = new ShaderCompiled(*_renderDevice, ShaderType::Vert,
		  "cshader/testUniformAndTexture.vert.spv");
	}
	if (_fragShad == nullptr) {
		_fragShad = new ShaderCompiled(*_renderDevice, ShaderType::Frag,
		  "cshader/testUniformAndTexture.frag.spv");
	}
	
	// Make layouts
	if (_dsl == nullptr) {
		std::vector<DescriptorLayoutBinding> dslb{};
		dslb.push_back(
		  DescriptorLayoutBinding{0, DescriptorDataType::UniformBuffer, 1,
			Flags<ShaderType>(ShaderType::Vert) | ShaderType::Frag});
		dslb.push_back(DescriptorLayoutBinding{
		  1, DescriptorDataType::ImageAndSampler, 1, ShaderType::Frag});
		_dsl = new DescriptorLayout(*_renderDevice, dslb);
	}
	// Make Pipeline
	_pipeline = new ShaderPipeline(*_renderDevice);
	_pipeline->bind(*_dsl);
	std::vector<const ShaderCompiled*> pointShad;
	pointShad.reserve(2);
	pointShad.push_back(_vertShad);
	pointShad.push_back(_fragShad);
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
inline void unloadSwapchain(bool remake = false) {
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
	unloadSwapchain(true);
	loadSwapchain(true);
}

static Image* _imgTest = nullptr;
static ImageView* _imgViewTest = nullptr;
static ImageSampler* _imgSamplerBasic = nullptr;

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
	// Make SwapChain and pipeline and programs and layouts
	loadSwapchain();
	// Test programe
	// Make vertex buffer
	_vertBuff = new VertexBuffer(
	  *_renderDevice, sizeof(testVert), MemoryFeature::IndirectWritable);
	_vertBuff->blockingIndirectWrite((void*)std::data(testVert));
	_indexBuff = new IndexBuffer(
	  *_renderDevice, sizeof(testInd), MemoryFeature::IndirectWritable);
	_indexBuff->blockingIndirectWrite((void*)std::data(testInd));

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(
	  "cshader/test.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;
	if (!pixels) throw "TEST_VulkanStbImageLoadFailure";

	_imgTest = new Image(*_renderDevice, uvec3{texWidth, texHeight, 1}, 2,
	  VK_FORMAT_R8G8B8A8_SRGB, TextureUseType::ShaderSampling,
	  MemoryFeature::IndirectWritable);
	assert(_imgTest.texMemorySize == imageSize);
	_imgTest->blockingIndirectWrite(pixels);
	stbi_image_free(pixels);
	_imgTest->blockingTransformActiveUsage(
	  TextureActiveUseType::ReadOnlyShader);

	_imgViewTest = new ImageView(*_renderDevice, *_imgTest);
	_imgSamplerBasic = new ImageSampler(
	  *_renderDevice, SamplerFilter::Nearest, SamplerFilter::Linear);
}

static RenderTick* _currentRenderTick = nullptr;
void vk::_prepareFrame() {
	auto& frame = _renderFrames[_currentFrame];
	if (frame != nullptr) {
		delete frame;
		frame = nullptr;
	}
	//checkStatus();
	frame = new RenderTick(*_renderDevice);
	_currentRenderTick = frame;
}
void vk::_sendFrame() {
	//checkStatus();
	_currentRenderTick->send();
	_currentFrame++;
	_currentFrame %= MAX_FRAMES_IN_FLIGHT;
	_newSwapchain = false;
}
void vk::_resetOutdatedFrame() { recreateSwapchain(); }

static std::vector<DescriptorSet> _ds{};
static std::vector<UniformBuffer> _ub{};
static const glm::vec4 START_COLOR{0.f, 0.5f, 0.7f, 1.f};
static glm::vec4 currentColor{1.f, 0.9f, 1.f, 1.f};

static DescriptorContainer* _dpc = nullptr;
inline void makeDescriptors(uint num) {
	if (_dpc != nullptr) delete _dpc;
	_dpc = new DescriptorContainer(
	  *_renderDevice, *_dsl, num, DescriptorPoolType::Dynamic);
}

void vk::setSwapchinCallback(SwapchainCallback callback) {
	scCallback = callback;
}

void vk::_testDraw() {
	if (_newSwapchain) {
		uint swapChainImageSize =
		  _renderDevice->swapChain->swapChainImages.size();
		makeDescriptors(swapChainImageSize);
		_commendBuffers.clear();
		_ds.clear();
		_ub.clear();
		_commendBuffers.reserve(swapChainImageSize);
		_ds.reserve(swapChainImageSize);
		_ub.reserve(swapChainImageSize);
		for (uint i = 0; i < swapChainImageSize; i++) {
			auto& ub = _ub.emplace_back(
			  *_renderDevice, sizeof(START_COLOR), MemoryFeature::Mappable);
			ub.directWrite(&START_COLOR);
			auto& ds = _ds.emplace_back(_dpc->allocNewSet());
			std::array<DescriptorSet::WriteData, 1> wd{
			  DescriptorSet::WriteData(&ub)};
			std::array<DescriptorSet::WriteData, 1> wd2{
			  DescriptorSet::WriteData(_imgViewTest, _imgSamplerBasic,
				TextureActiveUseType::ReadOnlyShader)};

			ds.blockingWrite(0, DescriptorDataType::UniformBuffer, 1, 0, wd);
			ds.blockingWrite(1, DescriptorDataType::ImageAndSampler, 1, 0, wd2);

			// CommendBuffers
			auto& cb = _commendBuffers.emplace_back(
			  _renderDevice->getCommendPool(CommendPoolType::Default));

			cb.startRecording(CommendBufferUsage::None);
			cb.cmdBeginRender(
			  *_renderPass, _frameBuffers.at(i), vec4(1., 0., 0., 0.));
			cb.cmdBind(*_pipeline);
			cb.cmdBind(ds, *_pipeline);
			cb.cmdBind(*_vertBuff);
			cb.cmdBind(*_indexBuff);
			// cb.cmdDraw((uint)std::size(testVert) / 2);

			cb.cmdDrawIndexed(std::size(testInd));
			cb.cmdEndRender();
			cb.endRecording();
		}
	}

	const float DELTA = 0.0001f;

	currentColor = util::transformHSV(currentColor, 0.1f, 1.f, 1.f);
	currentColor = glm::vec4(glm::normalize(glm::vec3{currentColor.x + DELTA,
							   currentColor.y + DELTA, currentColor.z + DELTA}),
	  1.f);

	_ub[_currentRenderTick->getImageIndex()].directWrite(&currentColor);

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
	cleanupFunc();	//????
	// testPrograme
	_commendBuffers.clear();
	_ub.clear();
	_ds.clear();
	if (_vertBuff != nullptr) delete _vertBuff;
	if (_indexBuff != nullptr) delete _indexBuff;
	if (_dpc != nullptr) delete _dpc;
	if (_dsl != nullptr) delete _dsl;
	if (_imgTest != nullptr) delete _imgTest;
	if (_imgViewTest != nullptr) delete _imgViewTest;
	if (_imgSamplerBasic != nullptr) delete _imgSamplerBasic;
	if (_vertShad != nullptr) delete _vertShad;
	if (_fragShad != nullptr) delete _fragShad;
	if (_renderDevice != nullptr) delete _renderDevice;
	// testPrograme end
	if (_OSSurface != nullptr) delete _OSSurface;
	#ifdef VULKAN_DEBUG
	if (_debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(_vk, _debugMessenger, nullptr);
	#endif
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
