module;
#include "Marco.h"
#	pragma warning(disable : 26812)
#	include <vulkan/vulkan.h>
#	include <assert.h>
module Vulkan;
import: Internal;
import: Enum;
import std.core;
import Define;
import Logger;
using namespace vk;

constexpr int RENDERTICK_INITIAL_MBR_SIZE = 4;

RenderTick::RenderTick(LogicalDevice& ld):
  d(ld), _completionFence(d), _presentSemaphore(d), _acquireSemaphore(d),
  _submitPools(RENDERTICK_INITIAL_MBR_SIZE) {
	_waitingForFence = false;
	_syncLines.emplace_back(TimelineSemaphore(d, 0), 0);
	outdated = false;
	imageIndex = -1;
	auto code = vkAcquireNextImageKHR(d.d, d.swapChain->sc, 0u,
	  _acquireSemaphore.sp, VK_NULL_HANDLE, &imageIndex);
	if (code == VK_ERROR_OUT_OF_DATE_KHR || code == VK_SUBOPTIMAL_KHR) {
		outdated = true;
		throw OutdatedSwapchainException();
	}
}
SyncPoint RenderTick::makeSyncLine(SyncNumber initialValue) {
	return _syncLines
	  .emplace_back(TimelineSemaphore(d, initialValue), initialValue)
	  .top();
}
void RenderTick::addCmdStage(CommendBuffer& cb,
  const std::vector<SyncPoint>& signalTo, const std::vector<SyncPoint>& waitFor,
  const std::vector<VkPipelineStageFlags>& waitType) {
	if constexpr (DO_SAFETY_CHECK) {
		if (waitFor.empty()) {
			if (waitType.size() != 1) throw "VulkanInvalidArgs";
		} else {
			if (waitFor.size() != waitType.size()) throw "VulkanInvalidArgs";
		}
	}

	auto* vkti = _submitPools.alloc<VkTimelineSemaphoreSubmitInfo>(1);
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
	  _submitPools.alloc<VkSemaphore>(waitFor.size() + signalTo.size());
	auto* syncNums = _submitPools.alloc<SyncNumber>(signalTo.size());
	auto* waitTypes = _submitPools.alloc<VkPipelineStageFlags>(waitType.size());
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
	if (signalTo.empty()) {
		sInfo.signalSemaphoreCount = 1;
		sInfo.pSignalSemaphores = &_presentSemaphore.sp;
	}
	if (waitFor.empty()) {
		sInfo.waitSemaphoreCount = 1;
		sInfo.pWaitSemaphores = &_acquireSemaphore.sp;
	}
	_cmdStages.push_back(sInfo);
}
SyncPoint RenderTick::getTopSyncPoint() {
	if constexpr (DO_SAFETY_CHECK)
		if (_syncPointStack.empty()) throw "VulkanSyncPointStackEmpty";
	return _syncPointStack.top();
}
void RenderTick::pushSyncPointStack(SyncPoint syncPoint) {
	_syncPointStack.push(syncPoint);
}
SyncPoint RenderTick::popSyncPointStack() {
	if constexpr (DO_SAFETY_CHECK)
		if (_syncPointStack.empty()) throw "VulkanSyncPointStackEmpty";
	return _syncPointStack.top();
}
void RenderTick::send() {
	if constexpr (DO_SAFETY_CHECK)
		if (!_syncPointStack.empty())
			logger("WARN: Vulkan RenderTick SyncPointStack not at base on "
				   "submitting call!\n");
	if (outdated) throw OutdatedSwapchainException();
	if (_cmdStages.empty()) { return;}

	if (vkQueueSubmit(
		  d.queue, _cmdStages.size(), _cmdStages.data(), _completionFence.fc)
		!= VK_SUCCESS) {
		outdated = true;
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	_waitingForFence = true;
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_presentSemaphore.sp;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &d.swapChain->sc;
	presentInfo.pImageIndices = &imageIndex;
	auto pCode = vkQueuePresentKHR(d.queue, &presentInfo);
	switch (pCode) {
	case VK_SUCCESS: break;
	case VK_SUBOPTIMAL_KHR:
	case VK_ERROR_OUT_OF_DATE_KHR:
		outdated = true;
		throw OutdatedSwapchainException();
		break;
	default: outdated = true; throw "VKQueuePresentUnknownError";
	}
}
void RenderTick::notifyOutdated() { outdated = true; }

bool RenderTick::isCompleted() const {
	if (!_waitingForFence) return true;
	if (vkGetFenceStatus(d.d, _completionFence.fc) == VK_NOT_READY)
		return false;
	return true;
}

bool RenderTick::waitForCompletion(ulint timeout) const {
	if (!_waitingForFence) return true;
	return (vkWaitForFences(d.d, 1, &_completionFence.fc, VK_TRUE, timeout)
			== VK_SUCCESS);
}
Buffer& RenderTick::makeStagingBuffer(size_t size) {
	return _stagingBuffers.emplace_back(d, size,
	  Flags(MemoryFeature::Mappable) | MemoryFeature::IndirectReadable);
}
CommendBuffer& RenderTick::makeSingleUseCommendBuffer(CommendPool& cp) {
	return _sigleUseCommendBuffer.emplace_back(cp);
}
void RenderTick::forceKill() {}
RenderTick::~RenderTick() {
	while (!waitForCompletion()) {};
	assert(isCompleted());
}