export module Vulkan: Commend;
export import: Declaration;
import: Enum;
import Define;
import Util;
import "VulkanExtModule.h";

// Commend class:
export namespace vk {

	/// @addtogroup vkCommend Commend
	/// @ingroup vulkan
	/// @brief A group of vulkan classes providing commend support
	///
	/// @todo A a group somehow to list all commend func?
	///
	/// @{

	/// A commend pool for allocating CommendBuffer
	class CommendPool: public ComplexObject
	{
		LogicalDevice& d;
		QueuePool& qp;

	  public:
		/// Make a commend pool that supports a queue group in a QueuePool.
		CommendPool(QueuePool& queuePool, uint queueGroupIndex,
		  Flags<CommendPoolType> type);
		CommendPool(const CommendPool&) = delete;
		CommendPool(CommendPool&& other) noexcept;
		/// @brief Destructor
		///
		/// All child CommendBuffer should be destructed before calling this
		/// destructor.
		/// @todo check if above is true
		~CommendPool();
		/// Make a commend buffer from this pool
		/// @todo Make a templated version for LifetimeManager support
		CommendBuffer makeCommendBuffer();
		/// @todo hide this
		VkCommandPool cp = nullptr;
		/// Get the underlying LogicalDevice
		LogicalDevice& getDevice() { return d; }
	};

	/// @brief A commend buffer for collecting commends
	class CommendBuffer: public ComplexObject
	{
	  public:
		/// @todo Move to private
		CommendBuffer(CommendPool& pool);
		CommendBuffer(const CommendBuffer&) = delete;
		CommendBuffer(CommendBuffer&& other) noexcept;
		/// Destructor
		~CommendBuffer();
		/// Start the recording of commends
		void startRecording(Flags<CommendBufferUsage> usage);
		/// Begin a \c 'render-state' using the provided RenderPass,
		/// FrameBuffer, and a background color
		void cmdBeginRender(
		  const RenderPass& rp, const FrameBuffer& fb, vec4 bgColor);
		/// Bind a RenderPipeline for later \c cmdDraw*() commends
		void cmdBind(const RenderPipeline& p);
		/// Bind a VertexBuffer for use as input to later \c 'cmdDraw*()'
		/// commends
		/// @todo Allow generic Buffer to be used here
		void cmdBind(
		  const VertexBuffer& vb, uint bindingPoint = 0, size_t offset = 0);
		/// Bind a IndexBuffer for use as input to later \c 'cmdDraw*()'
		/// commends
		/// @todo Allow generic Buffer to be used here
		void cmdBind(const IndexBuffer& ib,
		  VkIndexType dataType = _toIndexTpye::val<uint>(), size_t offset = 0);
		// TODO: add multibinding for descriptor sets
		/// Bund a DescriptorSet to a RenderPipeline for later \c 'cmdDraw*()'
		/// commends
		/// @param ds A descriptor set to bind.
		/// @param p The current active pipeline to bind to.
		/// @todo Check if this needs the CommendBuffer to be in \c
		/// 'render-state'
		void cmdBind(const DescriptorSet& ds, const RenderPipeline& p);
		/// Change the active state of an image.
		/// @todo Add params
		void cmdChangeState(Image& target, TextureSubRegion subRegion,
		  ImageRegionState start, Flags<PipelineStage> srcStage,
		  Flags<MemoryAccess> srcAccess, ImageRegionState end,
		  Flags<PipelineStage> dstStage, Flags<MemoryAccess> dstAccess);
		/// Set the push constants
		void cmdPushConstants(const RenderPipeline& p,
		  Flags<ShaderType> shaders, uint offset, uint size, const void* data);
		/// Start a draw commend
		void cmdDraw(
		  uint vCount, uint iCount = 1, uint vOffset = 0, uint iOffset = 0);
		/// Start a indexed draw commend
		void cmdDrawIndexed(uint indCount, uint insCount = 1,
		  uint indOffset = 0, uint vertOffset = 0, uint insOffset = 0);
		/// Copy a Buffer's content to another Buffer's content using indirect
		/// memory operations
		void cmdCopy(const Buffer& src, Buffer& dst, size_t size,
		  size_t srcOffset = 0, size_t dstOffset = 0);
		/// Copy a Buffer's content to an Image's sub layers using indirect
		/// memory operations
		void cmdCopy(const Buffer& src, Image& dst, ImageRegionState usage,
		  TextureSubLayers subLayers, uvec3 inputTextureSize,
		  size_t inputDataOffset, uvec3 copyRegion, ivec3 copyOffset);
		/// Copy an Image's sub layers to Buffer's content using indirect memory
		/// operations
		void cmdCopy(const Image& src, ImageRegionState usage, Buffer& dst,
		  TextureSubLayers subLayers, uvec3 copyRegion, ivec3 copyOffset,
		  uvec3 outputTextureSize, size_t outputDataOffset);
		/// End the \c 'render-state'
		void cmdEndRender();
		/// Complete the current recording
		void endRecording();
		/// Submit the buffer, and returns a waitable Fence object to wait on
		/// @warning Do not destruct the Fence, CommendBuffer, or anything that
		/// the CommendBuffer uses before the returned Fence object is signaled!
		Fence quickSubmit(Queue& queue);
		/// @todo hide this
		VkCommandBuffer cb = nullptr;
		/// @todo hide this
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


	///@}
}
