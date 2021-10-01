export module Vulkan: Enum;
import "VulkanExtModule.h";
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

// Lifetime
export namespace vk {}

// Memory
export namespace vk {
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
	template<> class Flags<vk::MemoryAccess>;
	enum class MemoryFeature {
		None = 0,
		IndirectWritable = 1,
		IndirectReadable = 2,
		Mappable = 4,
		Coherent = 8,
		CoherentMappable = Coherent | Mappable,
	};
	template<> class Flags<vk::MemoryFeature>;
}

// Device
export namespace vk {}

// Swapchain
export namespace vk {
	enum class SurfacePresentMode : std::underlying_type_t<VkPresentModeKHR> {
		Immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
		Mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
		Fifo = VK_PRESENT_MODE_FIFO_KHR,
		FifoRelaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
	};
	enum class SurfaceColorSpace : std::underlying_type_t<VkColorSpaceKHR> {
		SrgbNonLinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		// TODO: Add other color spaces (Extension required)
	};
	enum class SurfaceTransparentAction : VkCompositeAlphaFlagsKHR {
		RemoveAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		PreMultiplied = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		PostMultiplied = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		ExternCall = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

}

// Queue
export namespace vk {
	enum class QueueType : VkQueueFlags {
		Graphics = VK_QUEUE_GRAPHICS_BIT,
		Compute = VK_QUEUE_COMPUTE_BIT,
		MemoryTransfer = VK_QUEUE_TRANSFER_BIT,
	};
	template<> class Flags<vk::QueueType>;
}

// Buffer
export namespace vk {
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
	template<> class Flags<vk::BufferUseType>;
}

// Image
export namespace vk {
	// Texture Overall Properties
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
	template<> class Flags<vk::TextureUseType>;
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
	template<> class Flags<vk::TextureFeature>;
	enum class SampleCount : VkSampleCountFlags {
		Bit1 = VK_SAMPLE_COUNT_1_BIT,
		Bit2 = VK_SAMPLE_COUNT_2_BIT,
		Bit4 = VK_SAMPLE_COUNT_4_BIT,
		Bit8 = VK_SAMPLE_COUNT_8_BIT,
		Bit16 = VK_SAMPLE_COUNT_16_BIT,
		Bit32 = VK_SAMPLE_COUNT_32_BIT,
		Bit64 = VK_SAMPLE_COUNT_64_BIT,
	};
	template<> class Flags<vk::SampleCount>;
	enum class TextureAspect : VkImageAspectFlags {
		Color = VK_IMAGE_ASPECT_COLOR_BIT,
		Depth = VK_IMAGE_ASPECT_DEPTH_BIT,
		Stencil = VK_IMAGE_ASPECT_STENCIL_BIT,
		DepthStencil = Depth | Stencil,
		Metadata = VK_IMAGE_ASPECT_METADATA_BIT,
		Plane0 = VK_IMAGE_ASPECT_PLANE_0_BIT,
		Plane1 = VK_IMAGE_ASPECT_PLANE_1_BIT,
		Plane2 = VK_IMAGE_ASPECT_PLANE_2_BIT,
	};
	template<> class Flags<vk::TextureAspect>;

	// Texture Regional Properties
	enum class ImageActiveUsage : std::underlying_type_t<VkImageLayout> {
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

	// Sampler Properties
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
}

// Shader
export namespace vk {
	enum class ShaderType : VkShaderStageFlags {
		None = 0,
		Vert = VK_SHADER_STAGE_VERTEX_BIT,
		Geom = VK_SHADER_STAGE_GEOMETRY_BIT,
		Frag = VK_SHADER_STAGE_FRAGMENT_BIT,
		All = VK_SHADER_STAGE_ALL,
	};
	template<> class Flags<vk::ShaderType>;
}

// Sync
export namespace vk {}

// Commend
export namespace vk {
	enum class CommendPoolType : VkCommandPoolCreateFlags {
		Default = 0,
		Shortlived = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		Resetable = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		Protected = VK_COMMAND_POOL_CREATE_PROTECTED_BIT,
	};
	template<> class Flags<vk::CommendPoolType>;
	enum class CommendBufferUsage : VkCommandBufferUsageFlags {
		None = 0,
		Streaming = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		RenderPassOwned = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		ParallelSubmit = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	};
	template<> class Flags<vk::CommendBufferUsage>;
}

// Descriptor
export namespace vk {
	enum class DescriptorPoolType : VkDescriptorPoolCreateFlags {
		None = 0,
		Resetable = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		Dynamic = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		HostOnly = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE,
	};
	template<> class Flags<vk::DescriptorPoolType>;
	enum class DescriptorDataType : std::underlying_type_t<VkDescriptorType> {
		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		Image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
		ImageAndSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		// TODO: Add many more types of descriptor
	};
}

// Pipeline
export namespace vk {

	// Attachments
	enum class AttachmentReadOp : std::underlying_type_t<VkAttachmentLoadOp> {
		Read = VK_ATTACHMENT_LOAD_OP_LOAD,
		Clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
		Undefined = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	};
	enum class AttachmentWriteOp : std::underlying_type_t<VkAttachmentStoreOp> {
		Write = VK_ATTACHMENT_STORE_OP_STORE,
		Undefined = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	};
	enum class ColorComponents : VkColorComponentFlags {
		None = 0,
		Red = VK_COLOR_COMPONENT_R_BIT,
		Green = VK_COLOR_COMPONENT_G_BIT,
		Blue = VK_COLOR_COMPONENT_B_BIT,
		Alpha = VK_COLOR_COMPONENT_A_BIT,
		All = Red | Green | Blue | Alpha,
	};
	template<> class Flags<vk::ColorComponents>;

	// Blending
	enum class BlendOperator : std::underlying_type_t<VkBlendOp> {
		Add = VK_BLEND_OP_ADD,
		Subtract = VK_BLEND_OP_SUBTRACT,
		ReverseSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
		Min = VK_BLEND_OP_MIN,
		Max = VK_BLEND_OP_MAX,
		// TODO: Maybe add support for advanced blend op
		/* Provided by VK_EXT_blend_operation_advanced
		= VK_BLEND_OP_ZERO_EXT,
		= VK_BLEND_OP_SRC_EXT,
		= VK_BLEND_OP_DST_EXT,
		= VK_BLEND_OP_SRC_OVER_EXT,
		= VK_BLEND_OP_DST_OVER_EXT,
		= VK_BLEND_OP_SRC_IN_EXT,
		= VK_BLEND_OP_DST_IN_EXT,
		= VK_BLEND_OP_SRC_OUT_EXT,
		= VK_BLEND_OP_DST_OUT_EXT,
		= VK_BLEND_OP_SRC_ATOP_EXT,
		= VK_BLEND_OP_DST_ATOP_EXT,
		= VK_BLEND_OP_XOR_EXT,
		= VK_BLEND_OP_MULTIPLY_EXT,
		= VK_BLEND_OP_SCREEN_EXT,
		= VK_BLEND_OP_OVERLAY_EXT,
		= VK_BLEND_OP_DARKEN_EXT,
		= VK_BLEND_OP_LIGHTEN_EXT,
		= VK_BLEND_OP_COLORDODGE_EXT,
		= VK_BLEND_OP_COLORBURN_EXT,
		= VK_BLEND_OP_HARDLIGHT_EXT,
		= VK_BLEND_OP_SOFTLIGHT_EXT,
		= VK_BLEND_OP_DIFFERENCE_EXT,
		= VK_BLEND_OP_EXCLUSION_EXT,
		= VK_BLEND_OP_INVERT_EXT,
		= VK_BLEND_OP_INVERT_RGB_EXT,
		= VK_BLEND_OP_LINEARDODGE_EXT,
		= VK_BLEND_OP_LINEARBURN_EXT,
		= VK_BLEND_OP_VIVIDLIGHT_EXT,
		= VK_BLEND_OP_LINEARLIGHT_EXT,
		= VK_BLEND_OP_PINLIGHT_EXT,
		= VK_BLEND_OP_HARDMIX_EXT,
		= VK_BLEND_OP_HSL_HUE_EXT,
		= VK_BLEND_OP_HSL_SATURATION_EXT,
		= VK_BLEND_OP_HSL_COLOR_EXT,
		= VK_BLEND_OP_HSL_LUMINOSITY_EXT,
		= VK_BLEND_OP_PLUS_EXT,
		= VK_BLEND_OP_PLUS_CLAMPED_EXT,
		= VK_BLEND_OP_PLUS_CLAMPED_ALPHA_EXT,
		= VK_BLEND_OP_PLUS_DARKER_EXT,
		= VK_BLEND_OP_MINUS_EXT,
		= VK_BLEND_OP_MINUS_CLAMPED_EXT,
		= VK_BLEND_OP_CONTRAST_EXT,
		= VK_BLEND_OP_INVERT_OVG_EXT,
		= VK_BLEND_OP_RED_EXT,
		= VK_BLEND_OP_GREEN_EXT,
		= VK_BLEND_OP_BLUE_EXT, */
	};
	enum class BlendFactor : std::underlying_type_t<VkBlendFactor> {
		Zero = VK_BLEND_FACTOR_ZERO,
		One = VK_BLEND_FACTOR_ONE,
		SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
		OneMinusSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		DstColor = VK_BLEND_FACTOR_DST_COLOR,
		OneMinusDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
		OneMinusSrcAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
		OneMinusDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
		ConstColor = VK_BLEND_FACTOR_CONSTANT_COLOR,
		OneMinusConstColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
		ConstAlpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
		OneMinusConstAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
		SrcAlphaSaturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
		Src1Color = VK_BLEND_FACTOR_SRC1_COLOR,
		OneMinusSrc1Color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
		Src1Alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
		OneMinusSrc1Alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
	};

	enum class CompareOperator : std::underlying_type_t<VkCompareOp> {
		AlwaysFalse = VK_COMPARE_OP_NEVER,
		Less = VK_COMPARE_OP_LESS,
		Equal = VK_COMPARE_OP_EQUAL,
		LessOrEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
		Greater = VK_COMPARE_OP_GREATER,
		NotEqual = VK_COMPARE_OP_NOT_EQUAL,
		GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
		AlwaysTrue = VK_COMPARE_OP_ALWAYS,
	};
	enum class LogicOperator : std::underlying_type_t<VkLogicOp> {
		None = 0,
		Zeros = VK_LOGIC_OP_CLEAR,
		IAndD = VK_LOGIC_OP_AND,
		IAndNotD = VK_LOGIC_OP_AND_REVERSE,
		I = VK_LOGIC_OP_COPY,
		NotIAndD = VK_LOGIC_OP_AND_INVERTED,
		D = VK_LOGIC_OP_NO_OP,
		IXorD = VK_LOGIC_OP_XOR,
		IOrD = VK_LOGIC_OP_OR,
		INorD = VK_LOGIC_OP_NOR,
		IXnorD = VK_LOGIC_OP_EQUIVALENT,
		NotD = VK_LOGIC_OP_INVERT,
		IOrNotD = VK_LOGIC_OP_OR_REVERSE,
		NotI = VK_LOGIC_OP_COPY_INVERTED,
		NotIOrD = VK_LOGIC_OP_OR_INVERTED,
		INandD = VK_LOGIC_OP_NAND,
		Ones = VK_LOGIC_OP_SET,
	};

	// Polygon Settings
	enum class PolygonMode : std::underlying_type_t<VkPolygonMode> {
		Fill = VK_POLYGON_MODE_FILL,
		Line = VK_POLYGON_MODE_LINE,
		Point = VK_POLYGON_MODE_POINT,
		// TODO: add support for VK_POLYGON_MODE_FILL_RECTANGLE_NV
		ExtRectangleFill = VK_POLYGON_MODE_FILL_RECTANGLE_NV,
	};
	enum class PrimitiveTopology : std::underlying_type_t<VkPrimitiveTopology> {
		PointList = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		LineList = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		LineStrip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		TriangleStrip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		TriangleFan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		LineListAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
		LineStripAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
		TriangleListAdjacency =
		  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
		TriangleStripAdjacency =
		  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
		PatchList = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	};
	enum class CullMode : VkCullModeFlags {
		None = VK_CULL_MODE_NONE,
		Front = VK_CULL_MODE_FRONT_BIT,
		Back = VK_CULL_MODE_BACK_BIT,
	};
	template<> class Flags<vk::CullMode>;
	enum class FrontDirection : std::underlying_type_t<VkFrontFace> {
		CounterClockwise = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		Clockwise = VK_FRONT_FACE_CLOCKWISE,
	};
	enum class BufferInputRate : std::underlying_type_t<VkVertexInputRate> {
		PerVertex = VK_VERTEX_INPUT_RATE_VERTEX,
		PerInstance = VK_VERTEX_INPUT_RATE_INSTANCE,
	};

	// Stage
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
	template<> class Flags<vk::PipelineStage>;
}
