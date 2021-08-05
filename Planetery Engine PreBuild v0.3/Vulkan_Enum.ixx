module;
#include "Marco.h"
#pragma warning(disable : 26812)
#include <vulkan/vulkan.h>
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
		Image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
		ImageAndSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		// TODO: Add many more types of descriptor
	};
	enum class DescriptorPoolType : VkDescriptorPoolCreateFlags {
		None = 0,
		Resetable = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		Dynamic = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		HostOnly = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE,
	};

	enum class AttachmentReadOp : std::underlying_type_t<VkAttachmentLoadOp> {
		Read = VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
		Undefined = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	};
	enum class AttachmentWriteOp : std::underlying_type_t<VkAttachmentStoreOp> {
		Write = VK_ATTACHMENT_STORE_OP_STORE,
		Undefined = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	};

	enum class PipelineStage : VkPipelineStageFlags {
		None = VK_PIPELINE_STAGE_NONE_KHR,
		TopOfPipe = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		DrawIndirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		VertexInput = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
		ShaderVertex = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
		ShaderTessControl = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
		ShaderTessEval = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
		ShaderGeometry = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
		ShaderFragment = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		FragmentEarlyTest = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		FragmentLateTest = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		OutputAttachmentColor = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		ShaderCompute = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		Transfer = VK_PIPELINE_STAGE_TRANSFER_BIT,
		BottomOfPipe = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		HostMemoryAccess = VK_PIPELINE_STAGE_HOST_BIT,
		AnyGraphics = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		AnyCommands = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	};
	enum class MemoryAccess : VkAccessFlags {
		None = VK_ACCESS_NONE_KHR,
		DrawIndirectRead = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
		VertexInputIndexRead = VK_ACCESS_INDEX_READ_BIT,
		VertexInputAttributeRead = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		ShaderUniformRead = VK_ACCESS_UNIFORM_READ_BIT,
		AttachmentInputRead = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
		ShaderAnyRead = VK_ACCESS_SHADER_READ_BIT,
		ShaderAnyWrite = VK_ACCESS_SHADER_WRITE_BIT,
		AttachmentColorRead = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		AttachmentColorWrite = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		AttachmentDepthStencilRead =
		  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		AttachmentDepthStencilWrite =
		  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		TransferRead = VK_ACCESS_TRANSFER_READ_BIT,
		TransferWrite = VK_ACCESS_TRANSFER_WRITE_BIT,
		HostMemoryRead = VK_ACCESS_HOST_READ_BIT,
		HostMemoryWrite = VK_ACCESS_HOST_WRITE_BIT,
		AnyRead = VK_ACCESS_MEMORY_READ_BIT,
		AnyWrite = VK_ACCESS_MEMORY_WRITE_BIT,
	};

	enum class SamplerFilter : std::underlying_type_t<VkFilter> {
		Nearest = VK_FILTER_NEAREST,
		Linear = VK_FILTER_LINEAR,
		ExtCubic = VK_FILTER_CUBIC_IMG,	 // Needs extension: VK_EXT_filter_cubic
	};

	enum class SamplerMipmapMode : std::underlying_type_t<VkSamplerMipmapMode> {
		Nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		Linear = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	};
	enum class SamplerClampMode : std::underlying_type_t<VkSamplerAddressMode> {
		Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		BorderColor = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		ClampToMirroredEdge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
	};
	enum class SamplerBorderColor : std::underlying_type_t<VkBorderColor> {
		FloatTransparentBlack = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
		IntTransparentBlack = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
		FloatBlack = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		IntBlack = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		FloatWhite = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		IntWhite = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
		// Needs extension: VK_EXT_custom_border_color
		ExtFloatCustom = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT,
		// Needs extension: VK_EXT_custom_border_color
		ExtIntCustom = VK_BORDER_COLOR_INT_CUSTOM_EXT,
	};

	enum class TextureAspect {
		Color,
		Depth,
		Stencil,
		DepthStencil,
	};
	inline VkImageAspectFlags _toBase(TextureAspect v) {
		switch (v) {
		case TextureAspect::Color: return VK_IMAGE_ASPECT_COLOR_BIT;
		case TextureAspect::Depth: return VK_IMAGE_ASPECT_DEPTH_BIT;
		case TextureAspect::Stencil: return VK_IMAGE_ASPECT_STENCIL_BIT;
		case TextureAspect::DepthStencil:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default: throw "VulkanInvalidTextureAspect";
		}
	}

	enum class TextureActiveUseType : std::underlying_type_t<VkImageLayout> {
		Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
		General = VK_IMAGE_LAYOUT_GENERAL,
		AttachmentColor = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		AttachmentDepthStencil =
		  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		ReadOnlyAttachmentDepthStencil =
		  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		ReadOnlyShader = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		HostWritable = VK_IMAGE_LAYOUT_PREINITIALIZED,
		// Provided by VK_VERSION_1_1
		// VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL =
		// 1000117000,
		// VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL =
		// 1000117001, Provided by VK_VERSION_1_2
		// VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
		// VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL = 1000241001,
		// VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
		// VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL = 1000241003,
		// Provided by VK_KHR_swapchain
		Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		// Provided by VK_KHR_synchronization2
		ReadOnly = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,
		Attachment = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
	};

	enum class TextureUseType : VkImageUsageFlags {
		None = 0,
		TransferSrc = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		TransferDst = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		ShaderSampling = VK_IMAGE_USAGE_SAMPLED_BIT,
		ShaderStorage = VK_IMAGE_USAGE_STORAGE_BIT,
		AttachmentColor = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		AttachmentDepthStencil = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		AttachmentLazyAllocated = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
		AttachmentInput = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
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

	enum class TextureFeature : VkImageCreateFlags {
		None = 0,
		// VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
		// VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
		// VK_IMAGE_CREATE_SPARSE_ALIASED_BIT,
		MutableView = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
		CubeView = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		AliasMemory = VK_IMAGE_CREATE_ALIAS_BIT,
		// Provided by VK_VERSION_1_1
		// VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT,
		Array2DView = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,
		CompressedBlockView = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
		ViewUsageSupport = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT,
		Protected = VK_IMAGE_CREATE_PROTECTED_BIT,
		DisjointPlane = VK_IMAGE_CREATE_DISJOINT_BIT,
	};

	enum class MemoryFeature {
		None = 0,
		IndirectWritable = 1,
		IndirectReadable = 2,
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
export template<> class Flags<vk::TextureUseType>;
export template<> class Flags<vk::BufferUseType>;
export template<> class Flags<vk::TextureFeature>;
export template<> class Flags<vk::MemoryFeature>;
export template<> class Flags<vk::CommendBufferUsage>;
export template<> class Flags<vk::ShaderType>;
export template<> class Flags<vk::DescriptorPoolType>;
export template<> class Flags<vk::PipelineStage>;
export template<> class Flags<vk::MemoryAccess>;
