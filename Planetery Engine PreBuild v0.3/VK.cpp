#include "VK.h"

#include "Logger.h"
#include "ThreadEvents.h"

#include <array>
#include <algorithm>
#include <set>
#include <fstream>
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
class PhysicalDevice;
class OSRenderSurface;
class SwapChainSupport;
class SwapChain;
class ImageView;
typedef uint QueueFamilyIndex;
class SwapChainSupport
{
  public:
	SwapChainSupport() = default;
	SwapChainSupport(const PhysicalDevice& pd, const OSRenderSurface& surface);
	VkSurfaceFormatKHR getFormat() const;
	VkPresentModeKHR getPresentMode(bool preferRelaxedVBlank = false) const;
	uvec2 getSwapChainSize(uvec2 preferredSize = uvec2(uint(-1))) const;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
class LogicalDevice
{
  public:
	LogicalDevice(const PhysicalDevice& pd, QueueFamilyIndex queueFamilyIndex);
	LogicalDevice(const LogicalDevice&) = delete;
	LogicalDevice(LogicalDevice&&) noexcept;
	VkDevice d;
	VkQueue queue;
	std::unique_ptr<SwapChain> swapChain;
	VkDevice operator->() { return d; }
	~LogicalDevice();
};
class PhysicalDevice
{
  public:
	PhysicalDevice(
	  VkPhysicalDevice _d, OSRenderSurface* renderSurface = nullptr);
	PhysicalDevice(const PhysicalDevice&) = delete;
	PhysicalDevice(PhysicalDevice&&) noexcept;
	VkPhysicalDevice d;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceFeatures features;
	std::vector<VkQueueFamilyProperties> queueFamilies;
	std::list<LogicalDevice> devices;
	int rating;
	bool meetRequirements;
	SwapChainSupport swapChain;
	OSRenderSurface* renderOut = nullptr;
	QueueFamilyIndex getQueueFamily(
	  VkQueueFlags requirement, OSRenderSurface* displayOutput = nullptr);
	LogicalDevice* makeDevice(
	  VkQueueFlags requirement, OSRenderSurface* renderSurface = nullptr);
	VkPhysicalDevice operator->() { return d; }
	bool operator==(const PhysicalDevice& other) const { return d == other.d; }
	std::weak_ordering operator<=>(const PhysicalDevice& other) const {
		return rating <=> other.rating;
	}
	~PhysicalDevice();
};
class OSRenderSurface
{
  public:
	OSRenderSurface();
	OSRenderSurface(const OSRenderSurface&) = delete;
	OSRenderSurface(OSRenderSurface&&) = delete;
	VkSurfaceKHR surface;
	VkSurfaceKHR operator->() { return surface; }
	~OSRenderSurface();
};
class SwapChain
{
  public:
	SwapChain(const OSRenderSurface& surface, const SwapChainSupport& sp,
	  const LogicalDevice& device, uvec2 preferredSize,
	  bool transparentWindow = false);
	SwapChain(const SwapChain&) = delete;
	SwapChain(SwapChain&&) = delete;
	const LogicalDevice& d;
	VkSwapchainKHR sc;
	VkSurfaceFormatKHR surfaceFormat;
	uvec2 pixelSize;
	std::vector<VkImage> swapChainImages;
	ImageView getChainImageView(uint index);
	~SwapChain();
};

class ImageView
{
  public:
	ImageView(const LogicalDevice& d, VkImageViewCreateInfo createInfo);
	ImageView(const ImageView&) = delete;
	ImageView(ImageView&& other) noexcept;
	ImageView& operator=(const ImageView&) = delete;
	ImageView& operator=(ImageView&&) = default;
	~ImageView();
	VkImageView imgView;
	const LogicalDevice& d;
};

enum class ShaderType {
	Vert,
	Geom,
	Frag,
};
inline VkShaderStageFlagBits _getVal(ShaderType st) {
	switch (st) {
	case ShaderType::Vert: return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderType::Geom: return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderType::Frag: return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	assert(false);
}

class ShaderCompiled
{
  public:
	ShaderCompiled(const LogicalDevice& d, ShaderType shaderType,
	  const std::string_view& file_name);
	ShaderCompiled(const ShaderCompiled&) = delete;
	ShaderCompiled(ShaderCompiled&& other) noexcept;
	~ShaderCompiled();
	VkPipelineShaderStageCreateInfo getCreateInfo() const;
	ShaderType shaderType;
	VkShaderModule sm;
	const LogicalDevice& d;
};

class RenderPass
{
  public:
	RenderPass(const LogicalDevice& d);
	RenderPass(const RenderPass& d) = delete;
	RenderPass(RenderPass&& d) noexcept;
	~RenderPass();
	void complete();
	VkRenderPass rp = nullptr;
	std::vector<VkAttachmentDescription> attachmentTypes;
	std::vector<std::vector<uint>> subpasses;
	const LogicalDevice& d;
};

class ShaderPipeline
{
  public:
	ShaderPipeline(const LogicalDevice& device);
	ShaderPipeline(const ShaderPipeline&) = delete;
	ShaderPipeline(ShaderPipeline&& other) noexcept;
	~ShaderPipeline();
	void complete(std::vector<const ShaderCompiled*> shaderModules,
	  VkViewport viewport, const RenderPass& renderPass);
	VkPipeline p = nullptr;
	VkPipelineLayout pl = nullptr;
	const LogicalDevice& d;
};

namespace _toFormat {
	template<typename T>
	constexpr VkFormat val();
	constexpr VkFormat val<float>() {
		return VK_FORMAT_R32_SFLOAT;
	}
	constexpr VkFormat val<vec2>() {
		return VK_FORMAT_R32G32_SFLOAT;
	}
	constexpr VkFormat val<vec3>() {
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	constexpr VkFormat val<vec4>() {
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	constexpr VkFormat val<double>() {
		return VK_FORMAT_R64_SFLOAT;
	}
	constexpr VkFormat val<dvec2>() {
		return VK_FORMAT_R64G64_SFLOAT;
	}
	constexpr VkFormat val<dvec3>() {
		return VK_FORMAT_R64G64B64_SFLOAT;
	}
	constexpr VkFormat val<dvec4>() {
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	}
	constexpr VkFormat val<uint>() {
		return VK_FORMAT_R32_SUNSIGNED_INT;
	}
	constexpr VkFormat val<uvec2>() {
		return VK_FORMAT_R32G32_SUNSIGNED_INT;
	}
	constexpr VkFormat val<uvec3>() {
		return VK_FORMAT_R32G32B32_SUNSIGNED_INT;
	}
	constexpr VkFormat val<uvec4>() {
		return VK_FORMAT_R32G32B32A32_SUNSIGNED_INT;
	}
	constexpr VkFormat val<int>() {
		return VK_FORMAT_R32_SINT;
	}
	constexpr VkFormat val<ivec2>() {
		return VK_FORMAT_R32G32_SINT;
	}
	constexpr VkFormat val<ivec3>() {
		return VK_FORMAT_R32G32B32_SINT;
	}
	constexpr VkFormat val<ivec4>() {
		return VK_FORMAT_R32G32B32A32_SINT;
	}
}

class Buffer
{
public:
	Buffer(const LogicalDevice& d, VkBufferCreateInfo bufferInfo);
	Buffer(const LogicalDevice& d, size_t size);
	Buffer(const Buffer&) = delete;
	Buffer(Buffer&& other);
	~Buffer();
	void write(size_t size, size_t offset, void* data);
	void* map(bool writeOnly = true);
	void unmap(bool flushData = true);
	size_t size;
	VkBuffer b = nullptr;
	VkMappedMemoryRange* mappedMemory = nullptr;
	const LogicalDevice& d;
};
class VertexBuffer
{
	VertexBuffer(size_t size, void* data = nullptr);
};

class VertexAttribute
{
public:
	VertexAttribute();
	void addAttribute(uint offset, uint size, VkFormat);
	template<typename T>
	void addAttributeByType() {
		addLocation(_currentOffset, sizeof(T), __toFormat::val<T>());
	}
	std::vector<VkVertexInputAttributeDescription> attributes;
	std::vector<VkVertexInputBindingDescription> bindingPoints;
	uint strideSize = 0;
	void addBindingPoint(uint fromAttributeIndex, uint toAttributeIndex);
	VkPipelineVertexInputStateCreateInfo getStructForPipeline();
};

class FrameBuffer
{
  public:
	FrameBuffer(const LogicalDevice& device);
	FrameBuffer(const FrameBuffer&) = delete;
	FrameBuffer(FrameBuffer&& other) noexcept;
	~FrameBuffer();

};


static bool _loadedPreInitData = false;
static std::vector<VkLayerProperties> _layersAvailable;
static std::vector<VkExtensionProperties> _extensionsAvailable;
static std::list<PhysicalDevice> _physicalDevices;
static OSRenderSurface* _OSSurface;
static std::vector<const char*> _layersEnabled;
static std::vector<const char*> _extensionsEnabled;
static VkDebugUtilsMessengerEXT _debugMessenger = nullptr;
static VkInstance _vk = nullptr;
static LogicalDevice* _renderDevice = nullptr;
static uint32_t glfwExtensionCount;
constexpr const std::pair<const char*, uint> LAYERS[] = {
  {"VK_LAYER_KHRONOS_validation", 0u},
};

constexpr const std::pair<const char*, uint> EXTENSIONS[] = {
  {"VK_EXT_debug_utils", 0u}, {"VK_EXT_validation_features", 0u},
  //{"VK_EXT_debug_report", 0u},
};

constexpr const char* DEVICE_EXTENSIONS[] = {
  "VK_KHR_swapchain",
};

LogicalDevice::LogicalDevice(const PhysicalDevice& pd, uint queueFamilyIndex) {
	static const float One = 1.0f;
	std::array<VkDeviceQueueCreateInfo, 1> queueInfo{};
	queueInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo[0].queueFamilyIndex = queueFamilyIndex;
	queueInfo[0].queueCount = 2;
	queueInfo[0].pQueuePriorities = &One;

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pQueueCreateInfos = queueInfo.data();
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pEnabledFeatures = &pd.features;
	deviceInfo.enabledExtensionCount = std::size(DEVICE_EXTENSIONS);
	deviceInfo.ppEnabledExtensionNames = std::data(DEVICE_EXTENSIONS);
	deviceInfo.flags = 0;
	if (vkCreateDevice(pd.d, &deviceInfo, nullptr, &d) != VK_SUCCESS) {
		logger("Vulkan failed to create main graphical device!\n");
		throw "VulkanCreateGraphicalDeviceFailure";
	}
	vkGetDeviceQueue(d, queueFamilyIndex, 0, &queue);

	// Make swapchain
	if (pd.renderOut != nullptr)
		swapChain = std::make_unique<SwapChain>(
		  *pd.renderOut, pd.swapChain, *this, uvec2{1100, 900});
}
LogicalDevice::LogicalDevice(LogicalDevice&& o) noexcept {
	queue = o.queue;
	d = o.d;
	o.d = nullptr;
}
LogicalDevice::~LogicalDevice() {
	if (d) vkDestroyDevice(d, nullptr);
}
PhysicalDevice::PhysicalDevice(
  VkPhysicalDevice _d, OSRenderSurface* renderSurface) {
	d = _d;
	meetRequirements = true;
	vkGetPhysicalDeviceProperties(d, &properties);
	vkGetPhysicalDeviceFeatures(d, &features);
	uint queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(d, &queueFamilyCount, nullptr);
	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
	  d, &queueFamilyCount, queueFamilies.data());

	renderOut = renderSurface;


	auto& limit = properties.limits;
	logger << properties.deviceName << "...";
	// GPU Requirements
	if (!features.geometryShader) meetRequirements = false;
	if (!features.tessellationShader) meetRequirements = false;
	{
		uint extensionCount;
		vkEnumerateDeviceExtensionProperties(
		  d, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(
		  d, nullptr, &extensionCount, availableExtensions.data());

		// OPTI: change it so that it sorts the avilb' extension before find
		// matches using std::lower_bound.
		for (const char* neededExt : DEVICE_EXTENSIONS) {
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
		swapChain = SwapChainSupport(*this, *renderSurface);
		if (swapChain.formats.empty() || swapChain.presentModes.empty())
			meetRequirements = false;
	}

	if (meetRequirements) logger << " Usable.";

	// GPU Ratings
	logger << " Rating: ";
	rating = 0;
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		rating += 1000;
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		rating += 100;
	rating += std::log(double(limit.maxComputeWorkGroupSize[0])
					   * double(limit.maxComputeWorkGroupSize[1])
					   * double(limit.maxComputeWorkGroupSize[2]))
			* 100;
	rating += limit.maxGeometryOutputVertices;
	rating += limit.maxGeometryTotalOutputComponents;
	rating += std::log(double(limit.sparseAddressSpaceSize)) * 100;

	logger << std::to_string(rating) << "\n";
	logger.closeLayer();
}
PhysicalDevice::PhysicalDevice(PhysicalDevice&& o) noexcept:
  devices(std::move(o.devices)), queueFamilies(std::move(o.queueFamilies)) {
	d = o.d;
	features = o.features;
	meetRequirements = o.meetRequirements;
	properties = o.properties;
	rating = o.rating;
	renderOut = o.renderOut;
	swapChain = o.swapChain;
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
LogicalDevice* PhysicalDevice::makeDevice(
  VkQueueFlags requirement, OSRenderSurface* renderSurface) {
	QueueFamilyIndex i = getQueueFamily(requirement, renderSurface);
	if (i == uint(-1)) return nullptr;
	return &devices.emplace_back(*this, i);
}
PhysicalDevice::~PhysicalDevice() {
	// Nothing to do here for now
}
OSRenderSurface::OSRenderSurface() {
	if (glfwCreateWindowSurface(_vk,
		  (GLFWwindow*)events::ThreadEvents::getGLFWWindow(), nullptr, &surface)
		!= VK_SUCCESS) {
		logger("Vulkan and GLFW failed to get OS specific Render Surface!\n");
		throw "VulkanGLFWCreateOSWindowSurfaceFailure";
	}
}
OSRenderSurface::~OSRenderSurface() {
	vkDestroySurfaceKHR(_vk, surface, nullptr);
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
SwapChain::SwapChain(const OSRenderSurface& surface, const SwapChainSupport& sp,
  const LogicalDevice& device, uvec2 preferredSize, bool transparentWindow):
  d(device) {
	surfaceFormat = sp.getFormat();
	auto presentMode = sp.getPresentMode(true);
	pixelSize = sp.getSwapChainSize(preferredSize);
	uint32_t bufferCount = std::min(sp.capabilities.maxImageCount - 1,
							 sp.capabilities.minImageCount + 2 - 1)
						 + 1;
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface.surface;
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
	createInfo.compositeAlpha = transparentWindow
								? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
								: VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(d.d, &createInfo, nullptr, &sc) != VK_SUCCESS) {
		logger("Vulkan failed to make Swapchain!\n");
		throw "VulkanGLFWCreateSwapchainFailure";
	}

	vkGetSwapchainImagesKHR(d.d, sc, &bufferCount, nullptr);
	swapChainImages.resize(bufferCount);
	vkGetSwapchainImagesKHR(d.d, sc, &bufferCount, swapChainImages.data());
}
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
SwapChain::~SwapChain() { vkDestroySwapchainKHR(d.d, sc, nullptr); }

ImageView::ImageView(const LogicalDevice& d, VkImageViewCreateInfo createInfo):
  d(d) {
	if (vkCreateImageView(d.d, &createInfo, nullptr, &imgView) != VK_SUCCESS) {
		logger("Vulkan failed to make Image View form VkImage!\n");
		throw "VulkanCreateImageViewFailure";
	}
}
ImageView::ImageView(ImageView&& other) noexcept: d(other.d) {
	imgView = other.imgView;
	other.imgView = nullptr;
}
ImageView::~ImageView() {
	if (imgView != nullptr) vkDestroyImageView(d.d, imgView, nullptr);
}
ShaderCompiled::ShaderCompiled(const LogicalDevice& device, ShaderType st,
  const std::string_view& file_name):
  d(device) {
	shaderType = st;
	std::vector<char> fBuffer;
	{
		std::ifstream file(file_name.data(), std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			logger(
			  "Vulkan failed to open compiled shader file: ", file_name, "\n");
			throw "VulkanOpenFileFailure";
		}
		size_t fileSize = (size_t)file.tellg();
		try {
			fBuffer.resize(fileSize);
		} catch (std::bad_alloc e) {
			logger("Vulkan Out-Of-Memory on opening compiled shader file: ",
			  file_name, " Bad_alloc_error.what(): ", e.what(), "\n");
			throw "VulkanOutOfMemoryFailure";
		}
		file.seekg(0);
		file.read(fBuffer.data(), fileSize);
		file.close();
	}
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fBuffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(fBuffer.data());
	if (vkCreateShaderModule(d.d, &createInfo, nullptr, &sm) != VK_SUCCESS) {
		logger("Vulkan Compiled shader file: ", file_name,
		  " cannot be parsed to a Shader Module. The file is maybe "
		  "corrupted.\n");
		throw "VulkanCompiledShaderFileParseFailure";
	}
}
ShaderCompiled::ShaderCompiled(ShaderCompiled&& other) noexcept: d(other.d) {
	sm = other.sm;
	other.sm = nullptr;
}
ShaderCompiled::~ShaderCompiled() {
	if (sm != nullptr) vkDestroyShaderModule(d.d, sm, nullptr);
}
VkPipelineShaderStageCreateInfo ShaderCompiled::getCreateInfo() const {
	VkPipelineShaderStageCreateInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	sInfo.stage = _getVal(shaderType);
	sInfo.module = sm;
	sInfo.pName = "main";
	return sInfo;
}
RenderPass::RenderPass(const LogicalDevice& device): d(device) {}
RenderPass::RenderPass(RenderPass&& o) noexcept: d(o.d) {
	rp = o.rp;
	o.rp = nullptr;
}
void RenderPass::complete() {
	std::vector<VkSubpassDescription> subpassDes;
	std::list<std::vector<VkAttachmentReference>> attRefs;
	subpassDes.reserve(attachmentTypes.size());
	for (uint i = 0; i < subpasses.size(); i++) {
		auto& attRef = attRefs.emplace_back();
		attRef.reserve(subpasses.at(i).size());
		for (uint j = 0; j < subpasses.at(i).size(); j++) {
			attRef.push_back(VkAttachmentReference{
			  .attachment = subpasses.at(i).at(j),
			  .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			});
		}
		subpassDes.push_back(VkSubpassDescription{
		  .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		  .colorAttachmentCount = (uint)attRef.size(),
		  .pColorAttachments = attRef.data(),
		});
	}
	VkRenderPassCreateInfo renderPassInfo{
	  .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	  .attachmentCount = (uint)attachmentTypes.size(),
	  .pAttachments = attachmentTypes.data(),
	  .subpassCount = (uint)subpassDes.size(),
	  .pSubpasses = subpassDes.data(),
	};
	if (vkCreateRenderPass(d.d, &renderPassInfo, nullptr, &rp) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}
RenderPass::~RenderPass() {
	if (rp != nullptr) vkDestroyRenderPass(d.d, rp, nullptr);
}

ShaderPipeline::ShaderPipeline(const LogicalDevice& device) : d(device) {}

ShaderPipeline::ShaderPipeline(ShaderPipeline&& other) noexcept: d(other.d) {}

ShaderPipeline::~ShaderPipeline() {
	if (pl != nullptr) vkDestroyPipelineLayout(d.d, pl, nullptr);
	if (p != nullptr) vkDestroyPipeline(d.d, p, nullptr);
}

void ShaderPipeline::complete(std::vector<const ShaderCompiled*> shaderModules,
  VkViewport viewport, const RenderPass& renderPass) {
	std::vector<VkPipelineShaderStageCreateInfo> sInfos;
	sInfos.reserve(shaderModules.size());
	for (auto& sm : shaderModules) sInfos.emplace_back(sm->getCreateInfo());

	VkPipelineVertexInputStateCreateInfo vertInputBinding{};
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(vec2);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	VkVertexInputAttributeDescription attributeDescription{};
	attributeDescription.binding = 0;
	attributeDescription.location = 0;
	attributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescription.offset = 0;
	vertInputBinding.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertInputBinding.vertexBindingDescriptionCount = 1;
	vertInputBinding.pVertexBindingDescriptions = &bindingDescription;
	vertInputBinding.vertexAttributeDescriptionCount = 1;
	vertInputBinding.pVertexAttributeDescriptions = &attributeDescription;

	VkPipelineInputAssemblyStateCreateInfo vertInputType{};
	vertInputType.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vertInputType.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	vertInputType.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = VkExtent2D{.width = (uint)viewport.width,
	  .height = (uint)viewport.height};	 // HACK HERE
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	// Optional
	rasterizer.depthBiasClamp = 0.0f;			// Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;		// Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;			 // Optional
	multisampling.pSampleMask = nullptr;			 // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;	 // Optional
	multisampling.alphaToOneEnable = VK_FALSE;		 // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
	  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
	  | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	colorBlendAttachment.dstColorBlendFactor =
	  VK_BLEND_FACTOR_ZERO;											 // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;			 // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;	 // Optional
	colorBlendAttachment.dstAlphaBlendFactor =
	  VK_BLEND_FACTOR_ZERO;								  // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;  // Optional
	colorBlending.sType =
	  VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;	 // Optional
	colorBlending.blendConstants[1] = 0.0f;	 // Optional
	colorBlending.blendConstants[2] = 0.0f;	 // Optional
	colorBlending.blendConstants[3] = 0.0f;	 // Optional

	// VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;			   // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;		   // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;	   // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr;  // Optional
	if (vkCreatePipelineLayout(d.d, &pipelineLayoutInfo, nullptr, &pl)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = sInfos.size();
	pipelineInfo.pStages = sInfos.data();
	pipelineInfo.pVertexInputState = &vertInputBinding;
	pipelineInfo.pInputAssemblyState = &vertInputType;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;	// Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;  // Optional
	pipelineInfo.layout = pl;
	pipelineInfo.renderPass =
	  renderPass.rp;  // TODO: add ref count in renderPass Ob
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
	pipelineInfo.basePipelineIndex = -1;			   // Optional
	if (vkCreateGraphicsPipelines(
		  d.d, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p)
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}


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
	for (const auto& p : LAYERS) {
		if (p.first == "") continue;
		if (!requestLayer(p.first, p.second)) {
			logger("Vulkan: Missing default requested layer: ", p.first,
			  "(minV:", p.second, ")\n");
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
	for (const auto& p : EXTENSIONS) {
		if (p.first == "") continue;
		if (!requestExtension(p.first, p.second)) {
			logger("Vulkan: Missing default requested extension: ", p.first,
			  "(minV:", p.second, ")\n");
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
	createInfo.enabledLayerCount = _layersEnabled.size();
	createInfo.ppEnabledLayerNames = _layersEnabled.data();
	createInfo.enabledExtensionCount = _extensionsEnabled.size();
	createInfo.ppEnabledExtensionNames = _extensionsEnabled.data();
	if (vkCreateInstance(&createInfo, nullptr, &_vk) != VK_SUCCESS) {
		logger("Failed to initialize Vulkan!\n");
		throw "VulkanCreateInstanceFailure";
	}
}
inline void createDebugger() {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
	  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
	  // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
	  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
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
	// logger.closeLayer();
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

static RenderPass* _renderPass = nullptr;
static ShaderPipeline* _pipeline = nullptr;

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

	// Make a test pipeline
	ShaderCompiled vertShad(
	  *_renderDevice, ShaderType::Vert, "cshader/point.cvert");
	ShaderCompiled fragShad(
	  *_renderDevice, ShaderType::Frag, "cshader/point.cfrag");
	_renderPass = new RenderPass(*_renderDevice);
	VkAttachmentDescription ad{
	  .format = _renderDevice->swapChain->surfaceFormat.format,
	  .samples = VK_SAMPLE_COUNT_1_BIT,
	  .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	  .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	  .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	  .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	  .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	  .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};
	_renderPass->attachmentTypes.push_back(ad);
	_renderPass->subpasses.emplace_back(1,0);
	_renderPass->complete();
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
	_pipeline->complete(pointShad, viewport, *_renderPass);
	assert(_pipeline->p != nullptr);
}

void vk::end() {
	logger("VK Interface end.\n");
	if (_debugMessenger)
		vkDestroyDebugUtilsMessengerEXT(_vk, _debugMessenger, nullptr);
	if (_vk) vkDestroyInstance(_vk, nullptr);
}
