export module Vulkan: Commend;
export import: Declaration;
import: Enum;
import Define;
import Util;
import "VulkanExtModule.h";

// Commend class:
export namespace vk {
	class CommendPool: public ComplexObject
	{
		LogicalDevice& d;
		QueuePool& qp;

	  public:
		CommendPool(QueuePool& queuePool, uint queueGroupIndex,
		  Flags<CommendPoolType> type);
		CommendPool(const CommendPool&) = delete;
		CommendPool(CommendPool&& other) noexcept;
		~CommendPool();
		CommendBuffer makeCommendBuffer();
		VkCommandPool cp = nullptr;
		LogicalDevice& getDevice() { return d; }
	};
	class CommendBuffer: public ComplexObject
	{
	  public:
		CommendBuffer(CommendPool& pool);
		CommendBuffer(const CommendBuffer&) = delete;
		CommendBuffer(CommendBuffer&& other) noexcept;
		~CommendBuffer();
		void startRecording(Flags<CommendBufferUsage> usage);
		void cmdBeginRender(
		  const RenderPass& rp, const FrameBuffer& fb, vec4 bgColor);
		void cmdBind(const RenderPipeline& p);
		void cmdBind(
		  const VertexBuffer& vb, uint bindingPoint = 0, size_t offset = 0);
		void cmdBind(const IndexBuffer& ib,
		  VkIndexType dataType = _toIndexTpye::val<uint>(), size_t offset = 0);
		// TODO: add multibinding for descriptor sets
		void cmdBind(const DescriptorSet& ds, const RenderPipeline& p);

		void cmdChangeState(Image& target, TextureSubRegion subRegion,
		  ImageRegionState start, Flags<PipelineStage> srcStage,
		  Flags<MemoryAccess> srcAccess, ImageRegionState end,
		  Flags<PipelineStage> dstStage, Flags<MemoryAccess> dstAccess);

		void cmdPushConstants(const RenderPipeline& p,
		  Flags<ShaderType> shaders, uint offset, uint size, const void* data);

		void cmdDraw(
		  uint vCount, uint iCount = 1, uint vOffset = 0, uint iOffset = 0);
		void cmdDrawIndexed(uint indCount, uint insCount = 1,
		  uint indOffset = 0, uint vertOffset = 0, uint insOffset = 0);
		void cmdCopy(const Buffer& src, Buffer& dst, size_t size,
		  size_t srcOffset = 0, size_t dstOffset = 0);
		void cmdCopy(const Buffer& src, Image& dst, ImageRegionState usage,
		  TextureSubLayers subLayers, uvec3 inputTextureSize,
		  size_t inputDataOffset, uvec3 copyRegion, ivec3 copyOffset);
		void cmdCopy(const Image& src, ImageRegionState usage, Buffer& dst,
		  TextureSubLayers subLayers, uvec3 copyRegion, ivec3 copyOffset,
		  uvec3 outputTextureSize, size_t outputDataOffset);

		void cmdEndRender();
		void endRecording();
		Fence quickSubmit(Queue& queue);
		VkCommandBuffer cb = nullptr;
		CommendPool& cp;
	};


	// TODO: Add back multi-commend submittions
	/*
	class CommendGroup
	{
		std::list<SyncLine> syncLines;
		std::vector<SyncPoint> syncPointStack;
		std::vector<VkSubmitInfo> cmdStages;
		LifetimeManager& lm;
	  public:
		CommendGroup(LifetimeManager& alloc);
		SyncPoint makeSyncLine(SyncNumber initialValue = 0);
		void addCmdStage(CommendBuffer& cb,
		  std::initializer_list<SyncPoint> signalTo,
		  std::initializer_list<SyncPoint> waitFor,
		  std::initializer_list<VkPipelineStageFlags> waitType);
		SyncPoint getTopSyncPoint();
		// void addSyncPointToLayer();
		// ^^^^Missing def... Forgot what this should do?
		void pushSyncPointStack(SyncPoint syncPoint);
		SyncPoint popSyncPointStack();

	}*/

}
