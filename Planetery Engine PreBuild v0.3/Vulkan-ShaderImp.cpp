module;
//This 'include' fixes redef error that happens if you use std::iostream... somehow...
#include <vulkan/vulkan.h>
module Vulkan: ShaderImp;
import: Shader;
import: Enum;
import: Device;
import std.core;
import Define;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
using namespace vk;

ShaderCompiled::ShaderCompiled(
  LogicalDevice& device, ShaderType st, const std::string_view& file_name):
  d(device) {
	shaderType = st;
	std::vector<char> fBuffer;
	{
		std::ifstream file{file_name.data(), file.ate | file.binary};
		if (!file.is_open() || !file.good()) {
			logger(
			  "Vulkan failed to open compiled shader file: ", file_name, "\n");
			throw "VulkanOpenFileFailure";
		}
		size_t fileSize = (size_t)file.tellg();
		logger(
		  "Vulkan: Reading file: ", file_name.data(), "(", fileSize, "B)...\n");
		try {
			fBuffer.resize(fileSize);
		} catch (std::bad_alloc e) {
			logger("Vulkan Out-Of-Memory on opening compiled shader file: ",
			  file_name, " Bad_alloc_error.what(): ", e.what(), "\n");
			throw "VulkanOutOfMemoryFailure";
		}
		file.seekg(0, file.beg);
		file.read(fBuffer.data(), fileSize);
		file.close();
	}
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = fBuffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(fBuffer.data());
	if (vkCreateShaderModule(d.d, &createInfo, nullptr, &sm) != VK_SUCCESS) {
		logger("Vulkan Compiled shader file: ", file_name, " (", fBuffer.size(),
		  "B)",
		  " cannot be parsed to a Shader Module. The file is maybe "
		  "corrupted.\n");
		throw "VulkanCompiledShaderFileParseFailure";
	}
}
ShaderCompiled::ShaderCompiled(ShaderCompiled&& other) noexcept: d(other.d) {
	shaderType = other.shaderType;
	sm = other.sm;
	other.sm = nullptr;
}
ShaderCompiled::~ShaderCompiled() {
	if (sm != nullptr) vkDestroyShaderModule(d.d, sm, nullptr);
}
VkPipelineShaderStageCreateInfo ShaderCompiled::getCreateInfo() const {
	VkPipelineShaderStageCreateInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	sInfo.stage = (VkShaderStageFlagBits)shaderType;
	sInfo.module = sm;
	sInfo.pName = "main";
	return sInfo;
}