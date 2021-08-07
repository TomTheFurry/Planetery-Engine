export module Vulkan: Pipeline;
export import: Internal;
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
	class RenderPass
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
		RenderPass(const RenderPass& d) = delete;
		RenderPass(RenderPass&& d) noexcept;
		~RenderPass();
		VkRenderPass rp = nullptr;
		std::vector<Attachment> attachments;
		std::vector<SubPass> subpasses;
		std::vector<VkSubpassDependency> subpassDependencies;
		LogicalDevice& d;
	};
	class ShaderPipeline
	{
	  public:
		ShaderPipeline(LogicalDevice& device);
		ShaderPipeline(const ShaderPipeline&) = delete;
		ShaderPipeline(ShaderPipeline&& other) noexcept;
		~ShaderPipeline();
		void bind(const DescriptorLayout& dsl);
		void complete(std::vector<const ShaderCompiled*> shaderModules,
		  VertexAttribute& va, VkViewport viewport,
		  const RenderPass& renderPass);
		VkPipeline p = nullptr;
		VkPipelineLayout pl = nullptr;
		LogicalDevice& d;

	  private:
		std::vector<VkDescriptorSetLayout> _dsl;
	};
}
