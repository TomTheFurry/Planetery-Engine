export module Vulkan: Pipeline;
export import: Declaration;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Pipeline class:
export namespace vk {
	class VertexAttribute
	{
	  public:
		VertexAttribute();
		void addAttribute(uint bindingPoint, uint size, VkFormat format);
		template<typename T> void addAttributeByType(uint bindingPoint = 0) {
			addAttribute(bindingPoint, sizeof(T), _toFormat::val<T>());
		}
		void addBindingPoint(uint stride = uint(-1),
		  BufferInputRate rate = BufferInputRate::PerVertex);
		std::vector<VkVertexInputAttributeDescription> attributes;
		std::vector<VkVertexInputBindingDescription> bindingPoints;
		uint strideSize = 0;
		VkPipelineVertexInputStateCreateInfo getStructForPipeline();
	};
	class RenderPass: public ComplexObject
	{
	  public:
		class Attachment: private VkAttachmentDescription
		{
			friend RenderPass;

		  public:
			Attachment(VkFormat colorFormat,
			  TextureActiveUseType currentActiveUsage, AttachmentReadOp readOp,
			  TextureActiveUseType outputActiveUsage, AttachmentWriteOp writeOp,
			  uint samples = 1);
			Attachment(VkFormat depthStencilformat,
			  TextureActiveUseType currentActiveUsage,
			  AttachmentReadOp depthReadOp, AttachmentReadOp stencilReadOp,
			  TextureActiveUseType outputActiveUsage,
			  AttachmentWriteOp depthWriteOp, AttachmentWriteOp stencilWriteOp,
			  uint samples = 1);
		};
		class SubPass
		{
			friend RenderPass;

		  public:
			SubPass(std::initializer_list<uint> inputColorAttachment,
			  std::initializer_list<uint> inputDepthStencilAttachment,
			  std::initializer_list<uint> outputColorAttachment,
			  std::initializer_list<uint> outputDepthStencilAttachment,
			  std::initializer_list<uint> outputResolveAttachment,
			  std::initializer_list<uint> forwardAttachment);

		  private:
			std::vector<VkAttachmentReference> input;
			std::vector<VkAttachmentReference> color;
			std::vector<VkAttachmentReference> depthStencil;
			std::vector<VkAttachmentReference> resolve;
			std::vector<uint> preserve;
			operator VkSubpassDescription() const;
		};
		class SubPassDependency: private VkSubpassDependency
		{
			friend RenderPass;

		  public:
			// spId: -1 = external
			SubPassDependency(uint spId, PipelineStage srcStage,
			  MemoryAccess srcAccess, PipelineStage dstStage,
			  MemoryAccess dstAccess, bool perPixelIndependent = true);
			// spId: -1 = external
			SubPassDependency(uint srcSpId, PipelineStage srcStage,
			  MemoryAccess srcAccess, uint dstSpId, PipelineStage dstStage,
			  MemoryAccess dstAccess, bool perPixelIndependent = true);
		};

		RenderPass(LogicalDevice& d,
		  std::initializer_list<Attachment> attachments,
		  std::initializer_list<SubPass> subPasses,
		  std::initializer_list<SubPassDependency> dependencies);
		RenderPass(LogicalDevice& d, std::span<const Attachment> attachments,
		  std::span<const SubPass> subPasses,
		  std::span<const SubPassDependency> dependencies);
		RenderPass(const RenderPass& d) = delete;
		RenderPass(RenderPass&& d) noexcept;
		~RenderPass();
		VkRenderPass rp = nullptr;
		std::vector<Attachment> attachments;
		std::vector<SubPass> subpasses;
		std::vector<VkSubpassDependency> subpassDependencies;
		LogicalDevice& d;

	  private:
		void _ctor(std::span<const Attachment> attachments,
		  std::span<const SubPass> subPasses,
		  std::span<const SubPassDependency> dependencies);
	};
	class RenderPipeline: public ComplexObject
	{
	  public:
		class PushConstantLayout: private VkPushConstantRange
		{
			friend class RenderPipeline;

		  public:
			PushConstantLayout() = default;
			PushConstantLayout(uint offset, uint size,
			  Flags<ShaderType> shaders = ShaderType::All);
		};

		class AttachmentBlending: private VkPipelineColorBlendAttachmentState
		{
			friend class RenderPipeline;

		  public:
			AttachmentBlending() {
				blendEnable = false;
				srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendOp = VK_BLEND_OP_ADD;
				srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				alphaBlendOp = VK_BLEND_OP_ADD;
				colorWriteMask =
				  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
				  | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			}
			AttachmentBlending(BlendFactor srcColorFactor,
			  BlendFactor dstColorFactor, BlendOperator colorOperator,
			  BlendFactor srcAlphaFactor, BlendFactor dstAlphaFactor,
			  BlendOperator alphaOperator,
			  Flags<ColorComponents> writeEnableMask) {
				blendEnable = true;
				srcColorBlendFactor = (VkBlendFactor)srcColorFactor;
				dstColorBlendFactor = (VkBlendFactor)dstColorFactor;
				colorBlendOp = (VkBlendOp)colorOperator;
				srcAlphaBlendFactor = (VkBlendFactor)srcAlphaFactor;
				dstAlphaBlendFactor = (VkBlendFactor)dstAlphaFactor;
				alphaBlendOp = (VkBlendOp)alphaOperator;
				colorWriteMask = writeEnableMask;
			}
		};
		class ShaderStage: private VkPipelineShaderStageCreateInfo
		{
			friend class RenderPipeline;

		  public:
			// TODO: Add support for Specialization Constants(???)
			ShaderStage(ShaderCompiled& shader, const char* entryName);
		};
		class DepthStencilSettings:
		  private VkPipelineDepthStencilStateCreateInfo
		{
			friend class RenderPipeline;

		  public:
			DepthStencilSettings(CompareOperator depthTestOperator,
			  bool depthWriting, std::optional<vec2> depthBounds) {
				sType =
				  VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
				pNext = nullptr;
				depthTestEnable =
				  depthWriting
				  || depthTestOperator != CompareOperator::AlwaysTrue;
				depthWriteEnable = depthWriting;
				depthCompareOp = (VkCompareOp)depthTestOperator;
				depthBoundsTestEnable = depthBounds.has_value();
				if (depthBounds.has_value()) {
					minDepthBounds = depthBounds->x;
					maxDepthBounds = depthBounds->y;
				}
				// TODO: Stencil Testing Settings
				stencilTestEnable = false;
			}
		};
		struct DepthBias {
			float constantFactor;
			float slopeFactor;
			float clamp;
		};
		// TODO: Add back the std::initializer_list func
		RenderPipeline(LogicalDevice& device,
		  std::span<DescriptorLayout* const> descriptorLayouts,
		  std::span<const PushConstantLayout> pushConstants,
		  RenderPass& renderPass, uint32_t subpassId,
		  std::span<const ShaderStage> stages, VertexAttribute& vertAttribute,
		  PrimitiveTopology vertTopology, bool primitiveRestartByIndex,
		  std::span<const VkViewport> viewports,
		  std::span<const VkRect2D> scissors, bool rasterizerClampDepth,
		  bool rasterizerDiscard, PolygonMode polygonMode,
		  Flags<CullMode> cullMode, FrontDirection frontDirection,
		  std::optional<DepthBias> depthBias, float lineWidth, uint sampleCount,
		  bool rasterizerSetAlphaToOne,
		  std::optional<DepthStencilSettings> depthStencilSettings,
		  LogicOperator colorBlendLogicOperator,
		  std::span<const AttachmentBlending> attachmentBlending,
		  vec4 blendConstants = vec4(1.f));
		RenderPipeline(const RenderPipeline&) = delete;
		RenderPipeline(RenderPipeline&& other) noexcept;
		~RenderPipeline();
		VkPipeline p = nullptr;
		VkPipelineLayout pl = nullptr;
		LogicalDevice& d;
	};
}
