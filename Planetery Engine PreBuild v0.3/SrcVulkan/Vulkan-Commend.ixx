export module Vulkan: Commend;
export import: Internal;
import: Enum;
import Define;
import Util;
import "VulkanExtModule.h";

// Commend class:
export namespace vk {
	class CommendPool
	{
	  public:
		CommendPool(LogicalDevice& device, CommendPoolType type);
		CommendPool(const CommendPool&) = delete;
		CommendPool(CommendPool&& other) noexcept;
		~CommendPool();
		CommendBuffer makeCommendBuffer();
		VkCommandPool cp = nullptr;
		LogicalDevice& d;
	};
	class CommendBuffer
	{
	  public:
		CommendBuffer(CommendPool& pool);
		CommendBuffer(const CommendBuffer&) = delete;
		CommendBuffer(CommendBuffer&& other) noexcept;
		~CommendBuffer();
		void startRecording(Flags<CommendBufferUsage> usage);
		void cmdBeginRender(
		  const RenderPass& rp, const FrameBuffer& fb, vec4 bgColor);
		void cmdBind(const ShaderPipeline& cb);
		void cmdBind(
		  const VertexBuffer& vb, uint bindingPoint = 0, size_t offset = 0);
		void cmdBind(const IndexBuffer& ib,
		  VkIndexType dataType = _toIndexTpye::val<uint>(), size_t offset = 0);
		void cmdBind(const DescriptorSet& ds,
		  const ShaderPipeline& p);	 // TODO: add multibinding

		void cmdChangeState(Image& target, TextureActiveUseType type,
		  Flags<PipelineStage> srcStage, Flags<MemoryAccess> srcAccess,
		  Flags<PipelineStage> dstStage, Flags<MemoryAccess> dstAccess);

		void cmdDraw(
		  uint vCount, uint iCount = 1, uint vOffset = 0, uint iOffset = 0);
		void cmdDrawIndexed(uint indCount, uint insCount = 1,
		  uint indOffset = 0, uint vertOffset = 0, uint insOffset = 0);
		void cmdCopy(const Buffer& src, Buffer& dst, size_t size,
		  size_t srcOffset = 0, size_t dstOffset = 0);
		void cmdCopy(const Buffer& src, Image& dst, TextureAspect aspect,
		  uvec3 inputSize, uvec3 copySize, ivec3 copyOffset = ivec3(0, 0, 0),
		  size_t inputOffset = 0, uint mipLevel = 0, uint layerOffset = 0,
		  uint layerCount = 1);
		void cmdCopy(Image& src, Buffer& dst, TextureAspect aspect,
		  uvec3 outputSize, uvec3 copySize, ivec3 copyOffset = ivec3(0, 0, 0),
		  size_t outputOffset = 0, uint mipLevel = 0, uint layerOffset = 0,
		  uint layerCount = 1);

		void cmdEndRender();
		void endRecording();
		Fence submit();
		VkCommandBuffer cb = nullptr;
		CommendPool& cp;
	};
}
