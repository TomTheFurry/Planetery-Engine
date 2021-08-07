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

	constexpr size_t RENDERTICK_INITIAL_MBR_SIZE = 512;
	class RenderTick
	{
	  public:
		RenderTick(LogicalDevice& d);
		RenderTick(const RenderTick&) = delete;
		RenderTick(RenderTick&&) = delete;
		SyncPoint makeSyncLine(SyncNumber initialValue = 0);
		void addCmdStage(CommendBuffer& cb,
		  const std::vector<SyncPoint>& signalTo,
		  const std::vector<SyncPoint>& waitFor,
		  const std::vector<VkPipelineStageFlags>& waitType);
		SyncPoint getTopSyncPoint();
		void addSyncPointToLayer();
		void pushSyncPointStack(SyncPoint syncPoint);
		SyncPoint popSyncPointStack();	// return layerEndingSyncPoint
		void send();
		void notifyOutdated();
		bool isOutdated() const { return outdated; }
		bool isStarted() const { return _waitingForFence; }
		uint getImageIndex() const { return imageIndex; }
		bool isCompleted() const;
		bool waitForCompletion(
		  ulint timeout = -1) const;  // return false for timeout
		Buffer& makeStagingBuffer(size_t size);
		CommendBuffer& makeSingleUseCommendBuffer(CommendPool& cp);
		void forceKill();
		~RenderTick();	// Will wait for completion
		LogicalDevice& d;

	  private:
		bool _waitingForFence;
		Fence _completionFence;
		Semaphore _acquireSemaphore;
		Semaphore _presentSemaphore;
		util::MBRPool _submitPools;
		std::list<SyncLine> _syncLines;
		std::stack<SyncPoint> _syncPointStack;
		std::vector<VkSubmitInfo> _cmdStages;
		std::list<Buffer> _stagingBuffers;
		std::list<CommendBuffer> _sigleUseCommendBuffer;
		uint imageIndex;
		bool outdated;
	};
}
