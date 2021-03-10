#include "VK.h"

#include "Logger.h"

#include <glfw/glfw3.h>

using namespace vk;

static VkInstance _vk;
std::vector<Extension> _vkExtension;

const std::vector<Extension>& vk::getExtensions() {
	return _vkExtension;
}

void vk::init() {
	logger("VK Interface init.\n");
	{
		uint count;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> ext(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, ext.data());
		_vkExtension.reserve(count);
		for (auto& ppt : ext) {
			_vkExtension.emplace_back(
			  Extension{std::string(ppt.extensionName), ppt.specVersion});
		}
	}
	if constexpr (IS_DEBUG_MODE) {
		logger.newLayer();
		logger << "Avalible VK Extensions: \n";
		for (auto& ex : _vkExtension) { logger(ex.name, ": ", ex.version);
		}
		logger.closeLayer();
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "-";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Planetery Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;
	VkResult result = vkCreateInstance(&createInfo, nullptr, &_vk);
	if (result != VK_SUCCESS) {
		logger("Failed to initialize Vulkan!\n");
		throw "VK init failed";
	}
}

void vk::end() {
	logger("VK Interface end.\n");
	vkDestroyInstance(_vk, nullptr);
}
