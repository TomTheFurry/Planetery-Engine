export module Vulkan: Tick;
export import: Internal;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

import: Sync;

// Tick class:
export namespace vk {
	struct OutdatedSwapchainException {};
	struct OutdatedFrameException {};

	constexpr size_t RENDERTICK_INITIAL_MBR_SIZE = 512;
	class RenderTick
	{
		friend class Swapchain;
		void reset(Semaphore&& as);	 // call from swapchain
		bool render();				 // call from swapchain, true: frame need resetting
		void send();				 // call from swapchain

	  public:
		static void setCallback(FrameCallback callback);
		RenderTick(
		  LogicalDevice& d, uint frameId, Semaphore&& acquireSemaphore);
		RenderTick(const RenderTick&) = delete;
		RenderTick(RenderTick&&) = delete;
		SyncPoint makeSyncLine(SyncNumber initialValue = 0);
		void addCmdStage(CommendBuffer& cb,
		  std::initializer_list<SyncPoint> signalTo,
		  std::initializer_list<SyncPoint> waitFor,
		  std::initializer_list<VkPipelineStageFlags> waitType);
		SyncPoint getTopSyncPoint();
		// void addSyncPointToLayer();
		// ^^^^Missing def... Forgot what this should do?
		void pushSyncPointStack(SyncPoint syncPoint);
		SyncPoint popSyncPointStack();	// return layerEndingSyncPoint
		void forceResetSwapchain();
		void forceResetFrame();	 // TODO: force reset container stuff
		bool isStarted() const { return frameSent; }
		uint getImageIndex() const { return imageIndex; }
		bool isCompleted() const;
		bool waitForCompletion(
		  ulint timeout = -1) const;  // return false for timeout
		Buffer& makeStagingBuffer(size_t size);
		CommendBuffer& makeSingleUseCommendBuffer(CommendPool& cp);
		~RenderTick();	// Will wait for completion
		LogicalDevice& d;

	  private:
		bool frameSent;
		bool outdated;
		uint imageIndex;
		std::list<Buffer> stagingBuffers;
		std::list<CommendBuffer> singleUseCommendBuffers;
		Fence completionFence;
		Semaphore presentSemaphore;
		Semaphore acquireSemaphore;
		util::MBRPool submitPools;
		std::list<SyncLine> syncLines;
		std::vector<SyncPoint> syncPointStack;
		std::vector<VkSubmitInfo> cmdStages;
	};
}
