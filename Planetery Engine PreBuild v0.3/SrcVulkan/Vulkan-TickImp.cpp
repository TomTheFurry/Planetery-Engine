module Vulkan: TickImp;
import: Tick;
import: Enum;
import: Device;
import: Commend;
import: Buffer;
import std.core;
import Define;
import Util;
import Logger;
import "Assert.h";
import "VulkanExtModule.h";
using namespace vk;

constexpr int RENDERTICK_INITIAL_MBR_SIZE = 4;

static FrameCallback frameCallback{};
void RenderTick::setCallback(FrameCallback callback) {
	frameCallback = callback;
}

RenderTick::RenderTick(LogicalDevice& d, uint frameId, Semaphore&& as):
  d(d), completionFence(d), acquireSemaphore(std::move(as)),
  presentSemaphore(d), submitPools(RENDERTICK_INITIAL_MBR_SIZE) {
	frameSent = false;
	outdated = false;
	syncLines.emplace_back(TimelineSemaphore(d, 0), 0);
	imageIndex = frameId;
	if (frameCallback.onCreate) frameCallback.onCreate(*this);
	// logger("RenderTick ", imageIndex, " create");
}
void RenderTick::reset(Semaphore&& as) {
	// logger("RenderTick ", imageIndex, " reset");
	waitForCompletion();
	stagingBuffers.clear();
	singleUseCommendBuffers.clear();
	syncLines.clear();
	syncPointStack.clear();
	cmdStages.clear();
	submitPools.reset();
	completionFence.reset();
	acquireSemaphore = std::move(as);
	frameSent = false;
	outdated = false;
	syncLines.emplace_back(TimelineSemaphore(d, 0), 0);
}
bool vk::RenderTick::render() {
	try {
		if (frameCallback.onDraw) frameCallback.onDraw(*this);
		return true;
	} catch (OutdatedFrameException) { return false; }
}

SyncPoint RenderTick::makeSyncLine(SyncNumber initialValue) {
	return syncLines
	  .emplace_back(TimelineSemaphore(d, initialValue), initialValue)
	  .top();
}
void RenderTick::addCmdStage(CommendBuffer& cb,
  std::initializer_list<SyncPoint> signalTo,
  std::initializer_list<SyncPoint> waitFor,
  std::initializer_list<VkPipelineStageFlags> waitType) {
	if constexpr (DO_SAFETY_CHECK) {
		if (std::empty(waitFor)) {
			if (waitType.size() != 1) throw "VulkanInvalidArgs";
		} else {
			if (waitFor.size() != waitType.size()) throw "VulkanInvalidArgs";
		}
	}

	auto* vkti = submitPools.alloc<VkTimelineSemaphoreSubmitInfo>(1);
	vkti->sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
	vkti->pNext = NULL;
	vkti->waitSemaphoreValueCount = waitFor.size();
	vkti->signalSemaphoreValueCount = signalTo.size();
	VkSubmitInfo sInfo;
	sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sInfo.pNext = vkti;
	sInfo.waitSemaphoreCount = waitFor.size();
	sInfo.commandBufferCount = 1;
	sInfo.pCommandBuffers = &cb.cb;
	sInfo.signalSemaphoreCount = signalTo.size();

	auto* semaphores =
	  submitPools.alloc<VkSemaphore>(waitFor.size() + signalTo.size());
	auto* syncNums = submitPools.alloc<SyncNumber>(signalTo.size());
	auto* waitTypes = submitPools.alloc<VkPipelineStageFlags>(waitType.size());
	std::copy(waitType.begin(), waitType.end(), waitTypes);
	sInfo.pWaitDstStageMask = waitTypes;

	vkti->pWaitSemaphoreValues = syncNums;
	sInfo.pWaitSemaphores = semaphores;
	for (auto& sp : waitFor) {
		*syncNums = sp.syncNumber;
		*semaphores = sp.syncLine->sp.sp;
		syncNums++;
		semaphores++;
	}
	vkti->pSignalSemaphoreValues = syncNums;
	sInfo.pSignalSemaphores = semaphores;
	for (auto& sp : signalTo) {
		*syncNums = sp.syncNumber;
		*semaphores = sp.syncLine->sp.sp;
		syncNums++;
		semaphores++;
	}
	if (std::empty(signalTo)) {
		sInfo.signalSemaphoreCount = 1;
		sInfo.pSignalSemaphores = &presentSemaphore.sp;
	}
	if (std::empty(waitFor)) {
		sInfo.waitSemaphoreCount = 1;
		sInfo.pWaitSemaphores = &acquireSemaphore.sp;
	}
	cmdStages.push_back(sInfo);
}
SyncPoint RenderTick::getTopSyncPoint() {
	if constexpr (DO_SAFETY_CHECK)
		if (syncPointStack.empty()) throw "VulkanSyncPointStackEmpty";
	return syncPointStack.back();
}
void RenderTick::pushSyncPointStack(SyncPoint syncPoint) {
	syncPointStack.push_back(syncPoint);
}
SyncPoint RenderTick::popSyncPointStack() {
	if constexpr (DO_SAFETY_CHECK)
		if (syncPointStack.empty()) throw "VulkanSyncPointStackEmpty";
	return syncPointStack.back();
}
void RenderTick::forceResetSwapchain() {
	throw OutdatedSwapchainException();
}
void vk::RenderTick::forceResetFrame() { throw OutdatedFrameException(); }
void RenderTick::send() {
	// logger("RenderTick ", imageIndex, " send");
	if constexpr (DO_SAFETY_CHECK)
		if (!syncPointStack.empty())
			logger("WARN: Vulkan RenderTick SyncPointStack not at base on "
				   "submitting call!\n");
	if (outdated) throw OutdatedSwapchainException();
	if (cmdStages.empty()) { return; }

	if (vkQueueSubmit(
		  d.queue, cmdStages.size(), cmdStages.data(), completionFence.fc)
		!= VK_SUCCESS) {
		outdated = true;
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	frameSent = true;
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentSemaphore.sp;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &d.swapChain->sc;
	presentInfo.pImageIndices = &imageIndex;
	auto pCode = vkQueuePresentKHR(d.queue, &presentInfo);
	switch (pCode) {
	case VK_SUCCESS: break;
	case VK_SUBOPTIMAL_KHR: logger("Note: SubotimalKhr noted.");
	case VK_ERROR_OUT_OF_DATE_KHR:
		// logger("Note: OutOfDateKhr noted.");
		outdated = true;
		throw OutdatedSwapchainException();
		break;
	default: outdated = true; throw "VKQueuePresentUnknownError";
	}
}

bool RenderTick::isCompleted() const {
	if (!frameSent) return true;
	if (vkGetFenceStatus(d.d, completionFence.fc) == VK_NOT_READY) return false;
	return true;
}

bool RenderTick::waitForCompletion(ulint timeout) const {
	if (!frameSent) return true;
	return (vkWaitForFences(d.d, 1, &completionFence.fc, VK_TRUE, timeout)
			== VK_SUCCESS);
}
Buffer& RenderTick::makeStagingBuffer(size_t size) {
	return stagingBuffers.emplace_back(d, size,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
}
CommendBuffer& RenderTick::makeSingleUseCommendBuffer(CommendPool& cp) {
	return singleUseCommendBuffers.emplace_back(cp);
}
RenderTick::~RenderTick() {
	while (!waitForCompletion()) {};
	if (frameCallback.onDestroy) frameCallback.onDestroy(*this);
	assert(isCompleted());
}