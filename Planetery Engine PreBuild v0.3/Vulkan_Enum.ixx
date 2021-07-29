module;
#include "Marco.h"
#	pragma warning(disable : 26812)
#	include <vulkan/vulkan.h>
export module Vulkan: Enum;
import std.core;
import Define;
import Util;
export namespace _toFormat {
	template<typename T> constexpr VkFormat val() {}
	template<> constexpr VkFormat val<float>() { return VK_FORMAT_R32_SFLOAT; }
	template<> constexpr VkFormat val<vec2>() {
		return VK_FORMAT_R32G32_SFLOAT;
	}
	template<> constexpr VkFormat val<vec3>() {
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	template<> constexpr VkFormat val<vec4>() {
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	template<> constexpr VkFormat val<double>() { return VK_FORMAT_R64_SFLOAT; }
	template<> constexpr VkFormat val<dvec2>() {
		return VK_FORMAT_R64G64_SFLOAT;
	}
	template<> constexpr VkFormat val<dvec3>() {
		return VK_FORMAT_R64G64B64_SFLOAT;
	}
	template<> constexpr VkFormat val<dvec4>() {
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	}
	template<> constexpr VkFormat val<uint>() { return VK_FORMAT_R32_UINT; }
	template<> constexpr VkFormat val<uvec2>() { return VK_FORMAT_R32G32_UINT; }
	template<> constexpr VkFormat val<uvec3>() {
		return VK_FORMAT_R32G32B32_UINT;
	}
	template<> constexpr VkFormat val<uvec4>() {
		return VK_FORMAT_R32G32B32A32_UINT;
	}
	template<> constexpr VkFormat val<int>() { return VK_FORMAT_R32_SINT; }
	template<> constexpr VkFormat val<ivec2>() { return VK_FORMAT_R32G32_SINT; }
	template<> constexpr VkFormat val<ivec3>() {
		return VK_FORMAT_R32G32B32_SINT;
	}
	template<> constexpr VkFormat val<ivec4>() {
		return VK_FORMAT_R32G32B32A32_SINT;
	}
}
export namespace _toIndexTpye {
	template<typename T> constexpr VkIndexType val() {}
	template<> constexpr VkIndexType val<uint>() {
		return VK_INDEX_TYPE_UINT32;
	}
	template<> constexpr VkIndexType val<usint>() {
		return VK_INDEX_TYPE_UINT16;
	}
}
export namespace vk {
	constexpr auto CommendPoolTypeCount = 3;
	enum class CommendPoolType : uint {
		Default = 0,
		Shortlived,
		Resetable,
		MAX_ENUM
	};
	enum class DescriptorDataType : std::underlying_type_t<VkDescriptorType> {
		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
	};

	enum class DescriptorPoolType : VkDescriptorPoolCreateFlags {
		None = 0,
		Resetable = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		Dynamic = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		HostOnly = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE,
	};
	enum class BufferUseType : VkBufferUsageFlags {
		None = 0,
		TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		UniformTexel = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
		StorageTexel = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
		Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		IndirectDraw = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		All = 511,
	};
	enum class MemoryFeature {
		None = 0,
		IndirectWritable = 1,
		IndirectCopyable = 2,
		Mappable = 4,
		Coherent = 8,
		CoherentMappable = Coherent | Mappable,
	};
	enum class CommendBufferUsage : VkCommandBufferUsageFlags {
		None = 0,
		Streaming = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		RenderPassOwned = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		ParallelSubmit = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	};
	enum class ShaderType : VkShaderStageFlags {
		None = 0,
		Vert = VK_SHADER_STAGE_VERTEX_BIT,
		Geom = VK_SHADER_STAGE_GEOMETRY_BIT,
		Frag = VK_SHADER_STAGE_FRAGMENT_BIT,
		All = VK_SHADER_STAGE_ALL,
	};
	enum class BufferInputRate : std::underlying_type_t<VkVertexInputRate> {
		PerVertex = VK_VERTEX_INPUT_RATE_VERTEX,
		PerInstance = VK_VERTEX_INPUT_RATE_INSTANCE,
	};
}
export template<> class Flags<vk::BufferUseType>;
export template<> class Flags<vk::MemoryFeature>;
export template<> class Flags<vk::CommendBufferUsage>;
export template<> class Flags<vk::ShaderType>;
export template<> class Flags<vk::DescriptorPoolType>;
