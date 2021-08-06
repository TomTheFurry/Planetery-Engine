export module Vulkan: Shader;
export import: Internal;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Shader class:
export namespace vk {
	class ShaderCompiled
	{
	  public:
		ShaderCompiled(LogicalDevice& d, ShaderType shaderType,
		  const std::string_view& file_name);
		ShaderCompiled(const ShaderCompiled&) = delete;
		ShaderCompiled(ShaderCompiled&& other) noexcept;
		~ShaderCompiled();
		VkPipelineShaderStageCreateInfo getCreateInfo() const;
		ShaderType shaderType;
		VkShaderModule sm;
		LogicalDevice& d;
	};
}
