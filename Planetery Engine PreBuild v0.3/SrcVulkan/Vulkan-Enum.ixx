export module Vulkan: Enum;
import "VulkanExtModule.h";
import std.core;
import Define;
import Util;


/// <summary>
/// for internal use
/// </summary>
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

/// <summary>
/// To be removed
/// </summary>
export namespace _toIndexTpye {
	template<typename T> constexpr VkIndexType val() {}
	template<> constexpr VkIndexType val<uint>() {
		return VK_INDEX_TYPE_UINT32;
	}
	template<> constexpr VkIndexType val<usint>() {
		return VK_INDEX_TYPE_UINT16;
	}
}

/// @addtogroup enum Enums
/// @ingroup vulkan

// Lifetime
export namespace vk {}

// Memory
export namespace vk {

	/// @addtogroup memoryEnum Memory Enums
	/// @ingroup enum
	/// @{

	/// @enum MemoryAccess
	/// <summary>
	/// Flag for Memory Access type
	/// </summary>
	/// This define what this section of memory can be used for. Setting as
	/// litte of them will increase performance.
	/// @note Has Flags version. See Flags<MemoryAccess>
	enum class MemoryAccess : VkAccessFlags {
		/// None needed
		None = VK_ACCESS_NONE_KHR,
		/// Read by indirect draw commend
		DrawIndirectRead = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
		/// Read as vertex data input by a pipeline
		VertexInputIndexRead = VK_ACCESS_INDEX_READ_BIT,
		/// Read as vertex input attribute by a pipeline
		VertexInputAttributeRead = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		/// Read as Uniform data by any shaders
		ShaderUniformRead = VK_ACCESS_UNIFORM_READ_BIT,
		/// Read as Input Attachment by a pipeline
		AttachmentInputRead = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
		/// Read as any type of data by any shaders
		ShaderAnyRead = VK_ACCESS_SHADER_READ_BIT,
		/// Written to as any type of data by any shaders
		ShaderAnyWrite = VK_ACCESS_SHADER_WRITE_BIT,
		/// Read as Color Attachment by a pipeline
		AttachmentColorRead = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
		/// Written to as Color Attachment by a pipeline
		AttachmentColorWrite = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		/// Read as Depth/Stencil Attachment by a pipeline
		AttachmentDepthStencilRead =
		  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
		/// Written to as Depth/Stencil Attachment by a pipeline
		AttachmentDepthStencilWrite =
		  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		/// Read by memory transfer commend
		TransferRead = VK_ACCESS_TRANSFER_READ_BIT,
		/// Written to by memory transfer commend
		TransferWrite = VK_ACCESS_TRANSFER_WRITE_BIT,
		/// Read by host memory via memory mapping
		HostMemoryRead = VK_ACCESS_HOST_READ_BIT,
		/// Written to by host memory via memory mapping
		HostMemoryWrite = VK_ACCESS_HOST_WRITE_BIT,
		/// Read via any valid means
		AnyRead = VK_ACCESS_MEMORY_READ_BIT,
		/// Written via any valid means
		AnyWrite = VK_ACCESS_MEMORY_WRITE_BIT,

	};

	/// @class Flags<MemoryAccess>
	/// <summary>
	/// Flags type of MemoryAccess
	/// </summary>
	/// See vk::MemoryAccess for availble flags.
	template<> class Flags<MemoryAccess>;

	/// @enum MemoryFeature
	/// <summary>
	/// Types of Memory Feature
	/// </summary>
	/// This control the allowed
	/// \ref CommendBuffer::cmdCopy() "memory transfer method"
	/// @note Has Flags version. See Flags<MemoryFeature>
	enum class MemoryFeature {
		/// None
		None = 0,
		/// Writable by memory transfer commends
		IndirectWritable = 1,
		/// Readable by commends
		IndirectReadable = 2,
		/// Mappable to host memory, Read/Write control is set via MemoryAccess
		/// flag
		Mappable = 4,
		/// Mapped memory is coherent. Any Read/Write does not have to be
		/// flushed
		Coherent = 8,
		/// Equal to Coherent \| Mappable
		CoherentMappable = Coherent | Mappable,
	};

	/// @class Flags<MemoryFeature>
	/// <summary>
	/// Flags type of MemoryFeature
	/// </summary>
	/// See vk::MemoryFeature for availble flags.
	template<> class Flags<MemoryFeature>;

	/// @}
}

// Device
export namespace vk {}

// Swapchain
export namespace vk {
	/// @addtogroup swapchainEnum Swachain Enums
	/// @ingroup enum
	/// @{

	/// @enum SurfacePresentMode
	/// <summary>
	/// Enum for OSRenderSurface present mode
	/// </summary>
	/// This define how the swapchain works, like what type of vsync \(or no
	/// vsync\)
	///
	/// <a
	/// href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPresentModeKHR.html#_description">
	/// See vulkan docs... </a>
	enum class SurfacePresentMode : std::underlying_type_t<VkPresentModeKHR> {
		/// No syncing at all. Present the image immediately. May cause screen
		/// tearing
		Immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
		/// Mailbox present mode. See vulkan docs above
		Mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
		/// Fifo present mode. See vulkan docs above
		Fifo = VK_PRESENT_MODE_FIFO_KHR,
		/// Relaxed Fifo present mode. See vulkan docs above
		FifoRelaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
	};

	/// @enum SurfaceColorSpace
	/// <summary>
	/// Enum for OSRenderSurface color space
	/// </summary>
	/// This refers to what the OSRenderSurface color space its in.
	/// @todo Add more color spaces
	///
	/// <a
	/// href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkColorSpaceKHR.html#_description">
	/// See vulkan docs... </a>
	///
	/// @showrefs
	enum class SurfaceColorSpace : std::underlying_type_t<VkColorSpaceKHR> {
		/// Default SRGB Color Space
		SrgbNonLinear = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		// TODO: Add other color spaces (Extension required)
	};

	/// @enum SurfaceTransparentAction
	/// <summary>
	/// Enum for what action to do for the transparentcy channel
	/// </summary>
	/// This changes the action that applies when the SwapchainImage is
	/// presented to the native OSRenderSurface. Note that currently most
	/// Operating Systems only support RemoveAlpha.
	///
	/// <a
	/// href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkCompositeAlphaFlagBitsKHR.html#_description">
	/// See vulkan docs... </a>
	enum class SurfaceTransparentAction : VkCompositeAlphaFlagsKHR {
		/// Set alpha channel to 1.0. Supported by most type of OS
		RemoveAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		/// Notify no action is needed as alpha channel is pre-multiplied
		PreMultiplied = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		/// Notify multipying alpha channel is needed
		PostMultiplied = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		/// Use native calls setting
		ExternCall = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	/// @}
}

// Queue
export namespace vk {
	/// @addtogroup queueEnum Queue Enums
	/// @ingroup enum
	/// @{
	/// @enum QueueType
	/// <summary>
	/// Flag to designate what group of commends the Queue support
	/// </summary>
	/// @note Has Flags version. See Flags<QueueType>
	enum class QueueType : VkQueueFlags {
		Graphics = VK_QUEUE_GRAPHICS_BIT,
		Compute = VK_QUEUE_COMPUTE_BIT,
		MemoryTransfer = VK_QUEUE_TRANSFER_BIT,
	};

	/// @class Flags<QueueType>
	/// <summary>
	/// Flags type of QueueType
	/// </summary>
	/// See vk::QueueType for availble flags.
	template<> class Flags<QueueType>;

	/// @}
}

// Buffer
export namespace vk {
	/// @addtogroup bufferEnum Buffer Enums
	/// @ingroup enum
	/// @{
	/// @enum BufferUseType
	/// <summary>
	/// Flag to designate how this buffer can be used
	/// </summary>
	/// @note Has Flags version. See Flags<BufferUseType>
	enum class BufferUseType : VkBufferUsageFlags {
		/// None
		None = 0,
		/// Source for transfer commend
		TransferSrc = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		/// Destination for transfer commend
		TransferDst = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		/// Uniform texel buffer for shaders
		UniformTexel = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
		/// Storage texel buffer for shaders
		StorageTexel = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
		/// Uniform buffer for shaders
		Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		/// Storage buffer for shaders
		Storage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		/// Index buffer for draw commend
		Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		/// Vertex buffer for draw commend
		Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		/// Indirect buffer for indirect draw commend
		IndirectDraw = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
		/// All types
		All = 511,
	};

	/// @class Flags<BufferUseType>
	/// <summary>
	/// Flags type of BufferUseType
	/// </summary>
	/// See vk::BufferUseType for availble flags.
	template<> class Flags<BufferUseType>;

	/// @}
}

// Image
export namespace vk {
	/// @addtogroup imageEnum Image Enums
	/// @ingroup enum
	/// @{

	/// Texture Overall Properties

	/// @enum ImageUseType
	/// <summary>
	/// Flag to designate how this image can be used
	/// </summary>
	/// @note Has Flags version. See Flags<ImageUseType>
	enum class ImageUseType : VkImageUsageFlags {
		/// None
		None = 0,
		/// Source for transfer commend
		TransferSrc = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		/// Destination for transfer commend
		TransferDst = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		/// Image sampling by ImageView from shaders
		ShaderSampling = VK_IMAGE_USAGE_SAMPLED_BIT,
		/// Storage image for shaders
		ShaderStorage = VK_IMAGE_USAGE_STORAGE_BIT,
		/// As Color Attachment
		AttachmentColor = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		/// As Depth/Stencil Attachment
		AttachmentDepthStencil = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		/// Added on flag that mark the attachemnt as lazily allocated
		AttachmentLazyAllocated = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
		/// As Input Attachment
		AttachmentInput = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
	};
	/// @class Flags<ImageUseType>
	/// <summary>
	/// Flags type of ImageUseType
	/// </summary>
	/// See vk::ImageUseType for availble flags.
	template<> class Flags<ImageUseType>;

	/// @enum TextureFeature
	/// <summary>
	/// Flag to designate what image view can be created
	/// </summary>
	/// @note Has Flags version. See Flags<TextureFeature>
	enum class TextureFeature : VkImageCreateFlags {
		/// None
		None = 0,
		// VK_IMAGE_CREATE_SPARSE_BINDING_BIT,
		// VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT,
		// VK_IMAGE_CREATE_SPARSE_ALIASED_BIT,
		/// Mutable view
		MutableView = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
		/// Cube image view
		CubeView = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		/// Aliased underlying memory
		AliasMemory = VK_IMAGE_CREATE_ALIAS_BIT,
		// Provided by VK_VERSION_1_1
		// VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT,
		/// 2D array view
		Array2DView = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,
		/// Compressed block memory
		CompressedBlockView = VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT,
		/// @todo WTF is extended useage?
		ViewUsageSupport = VK_IMAGE_CREATE_EXTENDED_USAGE_BIT,
		/// Protected underlying memory
		Protected = VK_IMAGE_CREATE_PROTECTED_BIT,
		/// Disjointed memory
		DisjointPlane = VK_IMAGE_CREATE_DISJOINT_BIT,
	};
	/// @class Flags<TextureFeature>
	/// <summary>
	/// Flags type of TextureFeature
	/// </summary>
	/// See vk::TextureFeature for availble flags.
	template<> class Flags<TextureFeature>;

	/// @enum SampleCount
	/// <summary>
	/// Flag to designate sample count of the image
	/// </summary>
	/// @note Has Flags version. See Flags<SampleCount>
	enum class SampleCount : VkSampleCountFlags {
		Bit1 = VK_SAMPLE_COUNT_1_BIT,	 ///< 1
		Bit2 = VK_SAMPLE_COUNT_2_BIT,	 ///< 2
		Bit4 = VK_SAMPLE_COUNT_4_BIT,	 ///< 4
		Bit8 = VK_SAMPLE_COUNT_8_BIT,	 ///< 8
		Bit16 = VK_SAMPLE_COUNT_16_BIT,	 ///< 16
		Bit32 = VK_SAMPLE_COUNT_32_BIT,	 ///< 32
		Bit64 = VK_SAMPLE_COUNT_64_BIT,	 ///< 64
	};
	/// @class Flags<SampleCount>
	/// <summary>
	/// Flags type of SampleCount
	/// </summary>
	/// See vk::SampleCount for availble flags.
	template<> class Flags<SampleCount>;

	/// @enum TextureAspect
	/// <summary>
	/// Flag to designate an aspect of a texture
	/// </summary>
	/// @note Has Flags version. See Flags<TextureAspect>
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
	/// @class Flags<TextureAspect>
	/// <summary>
	/// Flags type of TextureAspect
	/// </summary>
	/// See vk::TextureAspect for availble flags.
	template<> class Flags<TextureAspect>;

	/// Texture Regional Properties

	/// @enum ImageRegionState
	/// <summary>
	/// Enum designating the active usage for (sections of) image
	/// </summary>
	/// To use Images in different actions/commends, you need to make sure the
	/// the accessed section of the image is in correct State.
	enum class ImageRegionState : std::underlying_type_t<VkImageLayout> {
		/// Undefined \(default initial state\)
		Undefined = VK_IMAGE_LAYOUT_UNDEFINED,
		/// General. Allow all type of actions
		General = VK_IMAGE_LAYOUT_GENERAL,
		/// Color Attachment
		AttachmentColor = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		/// Depth/Stencil Attachment
		AttachmentDepthStencil =
		  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		/// Read Only Depth/Stencil Attachment
		ReadOnlyAttachmentDepthStencil =
		  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		/// Read Only data from shaders
		ReadOnlyShader = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		/// Source for transfer commend
		TransferSrc = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		/// Destination for transfer commend
		TransferDst = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		/// Mappable and writable directly
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
		/// Prsentation to an OSRnderSurface
		Present = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		/// General read only
		/// @note Provided by VK_KHR_synchronization2
		ReadOnly = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR,
		/// General Attachment
		/// @note Provided by VK_KHR_synchronization2
		Attachment = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
	};

	/// Sampler Properties

	/// @enum SamplerFilter
	/// <summary>
	/// Enum setting the filter mode for a sampler
	/// </summary>
	enum class SamplerFilter : std::underlying_type_t<VkFilter> {
		/// Use nearest sample
		Nearest = VK_FILTER_NEAREST,
		/// Linear interpolation between samples
		Linear = VK_FILTER_LINEAR,
		/// Cubic interpolation between samples
		/// @note Requires VK_EXT_Filter_cuble Extension
		ExtCubic = VK_FILTER_CUBIC_IMG,
	};

	/// @enum SamplerMipmapMode
	/// <summary>
	/// Enum setting the mipmap mode for a sampler
	/// </summary>
	enum class SamplerMipmapMode : std::underlying_type_t<VkSamplerMipmapMode> {
		/// Use nearest mipmap level
		Nearest = VK_SAMPLER_MIPMAP_MODE_NEAREST,
		/// Linear interpolation between mipmap levels
		Linear = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	};

	/// @enum SamplerClampMode
	/// <summary>
	/// Enum setting the edge clamping mode for a sampler
	/// </summary>
	enum class SamplerClampMode : std::underlying_type_t<VkSamplerAddressMode> {
		/// Repeat the texture samples
		Repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		/// Repeat the texture samples, but in mirrored mode
		MirroredRepeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
		/// Clamp the sample to the edge
		ClampToEdge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		/// Use border color
		BorderColor = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		/// Clamp the sample to the edge in the oposite size
		ClampToMirroredEdge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
	};

	/// @enum SamplerBorderColor
	/// <summary>
	/// Enum setting the border color for a sampler
	/// </summary>
	enum class SamplerBorderColor : std::underlying_type_t<VkBorderColor> {
		/// Transparent black as float
		FloatTransparentBlack = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
		/// Transparent black as int
		IntTransparentBlack = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
		/// Black as float
		FloatBlack = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
		/// Black as int
		IntBlack = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		/// White as float
		FloatWhite = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		/// White as int
		IntWhite = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
		/// Custom float color
		/// @note Needs extension: VK_EXT_custom_border_color
		ExtFloatCustom = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT,
		/// Custom int color
		/// @note Needs extension: VK_EXT_custom_border_color
		ExtIntCustom = VK_BORDER_COLOR_INT_CUSTOM_EXT,
	};

	/// @}
}

// Shader
export namespace vk {
	/// @addtogroup shaderEnum Shader Enums
	/// @ingroup enum
	/// @{
	
	/// @enum ShaderType
	/// <summary>
	/// Flag to designate different shader type
	/// </summary>
	/// @note Has Flags version. See Flags<ShaderType>
	enum class ShaderType : VkShaderStageFlags {
		None = 0,
		Vert = VK_SHADER_STAGE_VERTEX_BIT,
		Geom = VK_SHADER_STAGE_GEOMETRY_BIT,
		Frag = VK_SHADER_STAGE_FRAGMENT_BIT,
		All = VK_SHADER_STAGE_ALL,
	};
	/// @class Flags<ShaderType>
	/// <summary>
	/// Flags type of ShaderType
	/// </summary>
	/// See vk::ShaderType for availble flags.
	template<> class Flags<ShaderType>;

	/// @}
}

// Sync
export namespace vk {}

// Commend
export namespace vk {
	/// @addtogroup commendEnum Commend Enums
	/// @ingroup enum
	/// @{
	
	/// @enum CommendPoolType
	/// <summary>
	/// Flag to designate different CommendPool type
	/// </summary>
	/// Control how the child CommendBuffer lifetime works
	/// @note Has Flags version. See Flags<CommendPoolType>
	enum class CommendPoolType : VkCommandPoolCreateFlags {
		/// Default
		Default = 0,
		/// CommendBuffer created from this pool is shortlived. Hint for Vulkan drivers for better optimization
		Shortlived = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		/// CommendBuffer created from this pool can be freed/reset. (Without resetting entire Commend Pool)
		Resetable = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		/// Using protected memory. @todo link to Protected
		Protected = VK_COMMAND_POOL_CREATE_PROTECTED_BIT,
	};
	/// @class Flags<CommendPoolType>
	/// <summary>
	/// Flags type of CommendPoolType
	/// </summary>
	/// See vk::CommendPoolType for availble flags.
	template<> class Flags<CommendPoolType>;

	/// @enum CommendBufferUsage
	/// <summary>
	/// Flag to designate how the CommendBuffer will be used
	/// </summary>
	/// Signal how this commend buffer is used. This allows better optimizations.
	/// @note Has Flags version. See Flags<CommendBufferUsage>
	enum class CommendBufferUsage : VkCommandBufferUsageFlags {
		/// None. Nothing special is needed
		None = 0,
		/// Streaming buffer. Each submition requires recording it again.
		/// @todo Need testing for statement: It will reset automatically after submition, not caring the parent CommendPool CommendPoolType::Resetable flag.
		Streaming = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		/// For Secondary buffer. Signal that this secondary buffer is entirely inside a render pass
		RenderPassOwned = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
		/// Allow parallel submition. Signal that this commend buffer can be submitted multiple times without waiting for previous usage be completed
		ParallelSubmit = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
	};
	/// @class Flags<CommendBufferUsage>
	/// <summary>
	/// Flags type of CommendBufferUsage
	/// </summary>
	/// See vk::CommendBufferUsage for availble flags.
	template<> class Flags<CommendBufferUsage>;

	/// @}
}

// Descriptor
export namespace vk {
	/// @addtogroup descriptorEnum Descriptor Enums
	/// @ingroup enum
	/// @{
	
	/// @enum DescriptorPoolType
	/// <summary>
	/// Flag to designate different DescriptorPool type
	/// </summary>
	/// Control how the child DescriptorSet lifetime works
	/// @note Has Flags version. See Flags<DescriptorPoolType>
	enum class DescriptorPoolType : VkDescriptorPoolCreateFlags {
		/// None. Nothing special
		None = 0,
		/// DescriptorSet created from this pool can be freed/reset. (Without resetting entire Descriptor Pool)
		Resetable = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		/// DescriptorSet created from this pool has bindings that can be updated after bind
		Dynamic = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		/// DescriptorSet created from this pool will be in host memory, allowing updating mutliple different DescriptorSets from multiple threads
		/// @note This still does not allow updating the same DescriptorSet from multiple threads!
		HostOnly = VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_VALVE,
	};
	/// @class Flags<DescriptorPoolType>
	/// <summary>
	/// Flags type of DescriptorPoolType
	/// </summary>
	/// See vk::DescriptorPoolType for availble flags.
	template<> class Flags<DescriptorPoolType>;

	/// @enum DescriptorDataType
	/// <summary>
	/// Enum to designate data type in a DescriptorBinding
	/// </summary>
	enum class DescriptorDataType : std::underlying_type_t<VkDescriptorType> {
		/// Store a reference to a UniformBuffer
		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		/// Store a reference to a StorageBuffer
		StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		/// Store an image view
		Image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		/// Store a sampler
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
		/// Store an image view and a sampler (combined)
		ImageAndSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		// TODO: Add many more types of descriptor
	};

	/// @}
}

// Pipeline
export namespace vk {
	/// @addtogroup pipelineEnum Pipeline Enums
	/// @ingroup enum
	/// @{

	/// Attachments
	
	/// @enum AttachmentReadOp
	/// <summary>
	/// Enum to set the read operation for an attachment
	/// </summary>
	/// Control when reading / inputing an attachment, what action can be done on the attachment
	/// @todo Better names? Perhaps LoadOp/InputOp instead of ReadOp?
	enum class AttachmentReadOp : std::underlying_type_t<VkAttachmentLoadOp> {
		/// Load the attachment data. Signal that the inputted value is needed and don\'t change it when loading in the attachment
		Read = VK_ATTACHMENT_LOAD_OP_LOAD,
		/// Clear the attachment data. Signal that while the value before is not needed/read, it should be cleared (set to a clear value)
		Clear = VK_ATTACHMENT_LOAD_OP_CLEAR,
		/// Don\'t care about the attachment data. Signal that the value before is not needed/read, and that the following write operations will override it entirely
		Undefined = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	};
	
	/// @enum AttachmentWriteOp
	/// <summary>
	/// Enum to set the write operation for an attachment
	/// </summary>
	/// Control when writing / outputing an attachment, what action can be done on the attachment
	/// @todo Better names? Perhaps StoreOp/OutputOp instead of WriteOp?
	enum class AttachmentWriteOp : std::underlying_type_t<VkAttachmentStoreOp> {
		/// Store the written attachment data. Signal that the attachment values will/may be read, and don't throw the value away
		Write = VK_ATTACHMENT_STORE_OP_STORE,
		/// Don\'t care about
		Undefined = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	};
	
	/// @enum ColorComponents
	/// <summary>
	/// Flag for color components in a color value
	/// </summary>
	enum class ColorComponents : VkColorComponentFlags {
		/// None
		None = 0,
		/// Red value
		Red = VK_COLOR_COMPONENT_R_BIT,
		/// Green value
		Green = VK_COLOR_COMPONENT_G_BIT,
		/// Blue value
		Blue = VK_COLOR_COMPONENT_B_BIT,
		/// Alpha value
		Alpha = VK_COLOR_COMPONENT_A_BIT,
		/// All values, including Red, Green, Blue, and Alpha
		All = Red | Green | Blue | Alpha,
	};
	/// @class Flags<ColorComponents>
	/// <summary>
	/// Flags type of ColorComponents
	/// </summary>
	/// See vk::ColorComponents for availble flags.
	template<> class Flags<ColorComponents>;

	/// Blending
	
	/// @enum BlendOperator
	/// <summary>
	/// Enum to set the blending operation
	/// </summary>
	/// Control how the blending is done.
	/// @todo: Add more for advanced blend op
	enum class BlendOperator : std::underlying_type_t<VkBlendOp> {
		/// RGBA: Destination += Source \(with weights from blend factor\)
		Add = VK_BLEND_OP_ADD,
		/// RGBA: Destination -= Source \(with weights from blend factor\)
		Subtract = VK_BLEND_OP_SUBTRACT,
		/// RGBA: Destination = Source - Destination \(with weights from blend factor\)
		ReverseSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
		/// RGBA: Destination = min(Source, Destination)
		Min = VK_BLEND_OP_MIN,
		/// RGBA: Destination = max(Source, Destination)
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
	
	/// @enum BlendFactor
	/// <summary>
	/// Enum to set the blending factor
	/// </summary>
	/// Effects the blending result based on the used BlendOperator
	enum class BlendFactor : std::underlying_type_t<VkBlendFactor> {
		/// 0.0
		Zero = VK_BLEND_FACTOR_ZERO,
		/// 1.0
		One = VK_BLEND_FACTOR_ONE,
		/// Respective channel value from source color
		SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
		/// One minus respective channel value from source color
		OneMinusSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
		/// Respective channel value from destination color
		DstColor = VK_BLEND_FACTOR_DST_COLOR,
		/// One minus respective channel value from destination color
		OneMinusDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
		/// Alpha channel value from source color
		SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
		/// One minus alpha channel value from source color
		OneMinusSrcAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		/// Alpha channel value from destination color
		DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
		/// One minus alpha channel value from destination color
		OneMinusDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
		/// Constant blend factor color
		ConstColor = VK_BLEND_FACTOR_CONSTANT_COLOR,
		/// One minus constant blend factor color
		OneMinusConstColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
		/// Alpha channel value from constant blend factor color
		ConstAlpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
		/// One minus alpha channel value from constant blend factor color
		OneMinusConstAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
		/// RGB: min(Source Alpha, 1 - Destination Alpha) \n
		/// Alpha: 1.0
		SrcAlphaSaturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
		/// Respective channel value from 2nd source color (used in dual source blending mode)
		Src1Color = VK_BLEND_FACTOR_SRC1_COLOR,
		/// One minus respective channel value from 2nd source color (used in dual source blending mode)
		OneMinusSrc1Color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
		/// Alpha channel value from 2nd source color (used in dual source blending mode)
		Src1Alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
		/// One minus alpha channel value from 2nd source color (used in dual source blending mode)
		OneMinusSrc1Alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
	};

	/// Stencil

	/// @enum CompareOperator
	/// <summary>
	/// Enum to set the stencil compare operator
	/// </summary>
	/// Control how stencil works
	enum class CompareOperator : std::underlying_type_t<VkCompareOp> {
		/// false
		AlwaysFalse = VK_COMPARE_OP_NEVER,
		/// A \< B
		Less = VK_COMPARE_OP_LESS,
		/// A == B
		Equal = VK_COMPARE_OP_EQUAL,
		/// A <= B
		LessOrEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
		/// A > B
		Greater = VK_COMPARE_OP_GREATER,
		/// A != B
		NotEqual = VK_COMPARE_OP_NOT_EQUAL,
		/// A >= B
		GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
		/// true
		AlwaysTrue = VK_COMPARE_OP_ALWAYS,
	};

	/// @enum LogicOperator
	/// <summary>
	/// Enum setting operator used on writing fragment value to color attachment
	/// </summary>
	/// Control what operator is applied when outputing to the color attachment

	/// @bug Need to disignate a value to turn off logic operator

	/// @todo Check the statement about blending is disabled when logic operation is turned on
	
	/// The following table assume that \'S\' == source value from fragment output, and \'d\' == destination value from the color attachment
	enum class LogicOperator : std::underlying_type_t<VkLogicOp> {
		/// Set to zero
		Zeros = VK_LOGIC_OP_CLEAR,
		/// S & D
		IAndD = VK_LOGIC_OP_AND,
		/// S & ~D
		IAndNotD = VK_LOGIC_OP_AND_REVERSE,
		/// S \(Copy\)
		I = VK_LOGIC_OP_COPY,
		/// ~S & D
		NotIAndD = VK_LOGIC_OP_AND_INVERTED,
		/// D \(No-op\)
		D = VK_LOGIC_OP_NO_OP,
		/// S ^ D
		IXorD = VK_LOGIC_OP_XOR,
		/// S | D
		IOrD = VK_LOGIC_OP_OR,
		/// ~ S|D
		INorD = VK_LOGIC_OP_NOR,
		/// ~ S^D
		IXnorD = VK_LOGIC_OP_EQUIVALENT,
		/// ~D
		NotD = VK_LOGIC_OP_INVERT,
		/// S | ~D
		IOrNotD = VK_LOGIC_OP_OR_REVERSE,
		/// ~S
		NotI = VK_LOGIC_OP_COPY_INVERTED,
		/// ~S | D
		NotIOrD = VK_LOGIC_OP_OR_INVERTED,
		/// ~ S&D
		INandD = VK_LOGIC_OP_NAND,
		/// Set to one
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
	template<> class Flags<CullMode>;
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
	template<> class Flags<PipelineStage>;
}
