module Vulkan: SwapchainImp;
import: Swapchain;
import: Device;
import: Enum;
import: Image;
import std.core;
import Define;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";

import ThreadEvents;
import "GlfwModule.h";
using namespace vk;

constexpr bool USE_MAILBOX_MODE = true;
constexpr auto PREFERRED_IMAGE_COUNT = 18u;


OSRenderSurface::Support::Support(
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
VkFormat OSRenderSurface::Support::getFormat() const {
	assert(!formats.empty());
	auto it = std::find_if(
	  formats.begin(), formats.end(), [](const VkSurfaceFormatKHR& v) {
		  return (v.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR
				  && v.format == VK_FORMAT_R8G8B8A8_SRGB);
	  });
	return it == formats.end() ? formats.front().format : it->format;
}
VkPresentModeKHR OSRenderSurface::Support::calculatePresentMode() const {
	if constexpr (USE_MAILBOX_MODE) return VK_PRESENT_MODE_MAILBOX_KHR;
	bool preferRelaxedVBlank = false;
	if (preferRelaxedVBlank
		&& std::find(presentModes.begin(), presentModes.end(),
			 VK_PRESENT_MODE_FIFO_RELAXED_KHR)
			 != presentModes.end())
		return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	else
		return VK_PRESENT_MODE_FIFO_KHR;
}
uvec2 OSRenderSurface::Support::calculateImageSize(uvec2 ps) const {
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
uint OSRenderSurface::Support::calculateImageCount(uint preferredCount) const {
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

OSRenderSurface::OSRenderSurface() {
	// TODO: Move this to seperate file... to save on compile time(?)
	if (glfwCreateWindowSurface(getVkInstance(),
		  (GLFWwindow*)events::ThreadEvents::getGLFWWindow(), nullptr, &surface)
		!= VK_SUCCESS) {
		logger("Vulkan and GLFW failed to get OS specific Render Surface!\n");
		throw "VulkanGLFWCreateOSWindowSurfaceFailure";
	}
}
OSRenderSurface::~OSRenderSurface() {
	sc.reset();
	vkDestroySurfaceKHR(getVkInstance(), surface, nullptr);
}

Swapchain& OSRenderSurface::querySwapchain(LogicalDevice& device) {
	if (sc && &(sc->getLogicalDevice()) == &device) {
		if (!sc->isValid()) sc->rebuild();
	} else
		sc.emplace(device, *this);
	return *sc;
}
void OSRenderSurface::releaseSwapchain() { sc.reset(); }

static SwapchainCallback swapchainCallback{};
void Swapchain::setCallback(SwapchainCallback sc) { swapchainCallback = sc; }
static FrameCallback frameCallback{};
void Swapchain::setCallback(FrameCallback fc) { frameCallback = fc; }

// NOTE: Three result from this _make() call:
// 1: return & Successful. sc != nullptr. *old_sc invalidated.
// 2: return & Failure. sc == nullptr. *old_sc invalidated.
// 3: throw Exception. *sc not touched. *old_sc not touched.
void Swapchain::_make() {
	VkSwapchainKHR old_sc = sc;
	auto sp = sf.querySupport(d.pd);
	surfaceFormat = sp.getFormat();
	auto presentMode = sp.calculatePresentMode();
	pixelSize = sp.calculateImageSize(sf.preferredImageSize);
	if (pixelSize == uvec2(0)) {
		logger("Vulkan: Noted that window size is 0. Halting rendering...\n");
		if (swapchainCallback.onSurfaceMinimized)
			swapchainCallback.onSurfaceMinimized(*this);
		throw SurfaceMinimizedException();
	}

	uint32_t bufferCount = sp.calculateImageCount(sf.preferredImageCount);
	SurfaceTransparentAction tType = sf.preferredTransparencyAction;
	if ((sp.capabilities.supportedCompositeAlpha
		  & (VkCompositeAlphaFlagBitsKHR)tType)
		== 0) {
		logger("Vulkan: WARNING: Window Surface does not support "
			   "VkCompositeAlphaFlagBitsKHR: ",
		  (VkCompositeAlphaFlagBitsKHR)tType, ". Setting it to default.\n");
		tType = SurfaceTransparentAction::RemoveAlpha;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = sf.surface;
	createInfo.minImageCount = bufferCount;
	createInfo.imageFormat = surfaceFormat;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent = VkExtent2D{pixelSize.x, pixelSize.y};
	createInfo.imageArrayLayers = 1;
	// FIXME: Hardcoded image usage (???)
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// TODO: Queue stuff
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = nullptr;
	createInfo.preTransform = sp.capabilities.currentTransform;
	createInfo.compositeAlpha = (VkCompositeAlphaFlagBitsKHR)tType;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = old_sc == nullptr ? VK_NULL_HANDLE : old_sc;
	auto result = vkCreateSwapchainKHR(d.d, &createInfo, nullptr, &sc);
	switch (result) {
	case VK_SUCCESS:
		vkGetSwapchainImagesKHR(d.d, sc, &bufferCount, nullptr);
		imgs.resize(bufferCount);
		vkGetSwapchainImagesKHR(d.d, sc, &bufferCount, imgs.data());
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
		// TODO: Maybe seperate destruction of images and delay the
		// onDestroy callback?
		scImgs.clear();
		perSwapchainResource.reset();
		if (swapchainCallback.onDestroy)
			swapchainCallback.onDestroy(*this, sc != nullptr);
		vkDestroySwapchainKHR(d.d, old_sc, nullptr);
	}
	if (sc != nullptr) {
		scImgs.resize(bufferCount);
		if (swapchainCallback.onCreate)
			swapchainCallback.onCreate(*this, old_sc != nullptr);
	}
}

Swapchain::Swapchain(LogicalDevice& device, OSRenderSurface& surface):
  d(device), sf(surface) {
	// logger("Create Swapchain...");
	sc = nullptr;
	_make();
}
void Swapchain::rebuild() {
	// logger("Rebuild Swapchain...");
	_make();
}
Swapchain::~Swapchain() {
	if (sc != nullptr) {
		scImgs.clear();
		perSwapchainResource.reset();
		if (swapchainCallback.onDestroy)
			swapchainCallback.onDestroy(*this, false);
		vkDestroySwapchainKHR(d.d, sc, nullptr);
	}
}

SwapchainImage* Swapchain::getNextImage(ulint timeout) {
	uint imageId = uint(-1);
	Semaphore sp{d};
	auto code =
	  vkAcquireNextImageKHR(d.d, sc, timeout, sp.sp, VK_NULL_HANDLE, &imageId);
	switch (code) {
	case VK_SUBOPTIMAL_KHR: outdated = true;
	case VK_SUCCESS: {
		auto& optPtr = scImgs.at(imageId);
		// logger("Swapchain Acquire ", imageId);
		if (optPtr) {
			// logger("Swapchain reset Tick ", imageId);
			optPtr->setImageAquired(std::move(sp));
		} else {
			// logger("Swapchain Make Tick ", imageId);
			optPtr.make(*this, imageId, imgs.at(imageId), std::move(sp));
		}
		return &*optPtr;
	}
	case VK_ERROR_OUT_OF_DATE_KHR: outdated = true; return nullptr;
	case VK_TIMEOUT: return nullptr;	// timeout!=0 && Image Not Ready
	case VK_NOT_READY: return nullptr;	// timeout==0 && Image Not Ready
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
bool Swapchain::renderNextImage(ulint timeout) {
	SwapchainImage* scImgPtr = getNextImage(timeout);
	if (scImgPtr == nullptr) return false;
	try {
		frameCallback.onDraw(*scImgPtr);
		return true;
	} catch (OutdatedSwapchainException e) {
		outdated = true;
		return false;
	}
}
// TODO: void Swapchain::invalidateAllImages() {}
void Swapchain::invalidateSwapchain() { outdated = true; }



void SwapchainImage::setImageAquired(Semaphore&& sp) {
	waitForCompletion();
	perFrameResource.reset();
	imgAquireSpOrCompleteFc = &perFrameResource.make<Semaphore>(std::move(sp));
}

SwapchainImage::SwapchainImage(
  Swapchain& _sc, uint _imgId, VkImage _img, Semaphore&& sp):
  sc(_sc),
  imgId(_imgId), img(_img) {
	imgAquireSpOrCompleteFc = &perFrameResource.make<Semaphore>(std::move(sp));
	if (frameCallback.onCreate) frameCallback.onCreate(*this);
}

SwapchainImage::~SwapchainImage() {
	waitForCompletion();
	perFrameResource.reset();
	perImageResource.reset();
	if (frameCallback.onDestroy) frameCallback.onDestroy(*this);
}

bool SwapchainImage::waitForCompletion(ulint timeout) const {
	if (!std::holds_alternative<Fence*>(imgAquireSpOrCompleteFc)) return true;
	return (vkWaitForFences(sc.getDevice().d, 1,
			  &std::get<Fence*>(imgAquireSpOrCompleteFc)->fc, VK_TRUE, timeout)
			== VK_SUCCESS);
}
// TODO void invalidateImage() {}

ImageView SwapchainImage::makeImageView() const {
	return ImageView(sc.getDevice(), _getImgViewInfo());
}

VkImageViewCreateInfo vk::SwapchainImage::_getImgViewInfo() const {
	VkImageViewCreateInfo cInfo{};
	cInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	cInfo.image = img;
	cInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	cInfo.format = getFormat();
	cInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	cInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	cInfo.subresourceRange.baseMipLevel = 0;
	cInfo.subresourceRange.levelCount = 1;
	cInfo.subresourceRange.baseArrayLayer = 0;
	cInfo.subresourceRange.layerCount = 1;
	return cInfo;
}
