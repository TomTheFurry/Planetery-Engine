module Vulkan: QueueImp;
import: Queue;
import: Device;
import: Enum;
import: Swapchain;
import: Commend;
import std.core;
import Define;
import Logger;
import "VulkanExtModule.h";
import "Assert.h";
constexpr size_t MAX_QUEUE_COUNT_PER_FAMILY = 4096;

using namespace vk;

// All values will be default inited to 0.
const std::array<float, MAX_QUEUE_COUNT_PER_FAMILY> constZeros{};


std::vector<VkDeviceQueueCreateInfo>
  QueuePoolLayout::getQueueCreateInfos() const {
	std::vector<VkDeviceQueueCreateInfo> qInfos{};
	qInfos.reserve(queueGroups.size());

	for (auto& [index, count] : queueGroups) {
		qInfos.push_back(VkDeviceQueueCreateInfo{
		  .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		  .pNext = nullptr,
		  .flags = 0,  // Note: Change this to allow protected Queues
		  .queueFamilyIndex = index,
		  .queueCount = count,
		  .pQueuePriorities = constZeros.data(),
		});
	}
	return qInfos;
}

QueuePoolLayout::QueuePoolLayout(const PhysicalDevice& phyicalDevice) {
	pd = phyicalDevice.d;
	uint familyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &familyCount, nullptr);
	std::vector<VkQueueFamilyProperties> qInfos{};
	qInfos.resize(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &familyCount, qInfos.data());
	assert(familyCount != 0);
	assert(familyCount < 64);  // There really shouldn't be more then 64 right?
	logger.newLayer();
	logger << "QueuePoolLayout: \n";
	queueFamilyProperties.reserve(familyCount);
	uint id = 0;
	for (auto& qInfo : qInfos) {
		auto& qp = queueFamilyProperties.emplace_back(
		  FamilySupport{Flags<QueueType>(qInfo.queueFlags), qInfo.queueCount});
		if (qp.allowedUsages.has(QueueType::Graphics)
			|| qp.allowedUsages.has(QueueType::Compute))
			qp.allowedUsages.set(QueueType::MemoryTransfer);
		logger << "QG: " << id << ", max: " << qInfo.queueCount
			   << ", flag: " << qInfo.queueFlags << "\n";
		id++;
	}
	logger.closeLayer();
}

Flags<QueueType> QueuePoolLayout::getFamilySupportType(uint familyIndex) const {
	if constexpr (DO_SAFETY_CHECK) {
		if (familyIndex >= queueFamilyProperties.size())
			throw "VulkanQueuePoolLayoutInvalidFamilyIndex";
	}
	return queueFamilyProperties.at(familyIndex).allowedUsages;
}

uint QueuePoolLayout::findFamilyBySupportType(
  Flags<QueueType> mustSupport, Flags<QueueType> mustNotSupport) const {
	for (uint i = 0; i < queueFamilyProperties.size(); ++i) {
		auto& fp = queueFamilyProperties[i];
		if (fp.allowedUsages.hasAll(mustSupport)
			&& !fp.allowedUsages.hasAny(mustNotSupport))
			return i;
	}
	return uint(-1);
}

uint vk::QueuePoolLayout::getFamilyMaxQueueCount(uint familyIndex) const {
	if constexpr (DO_SAFETY_CHECK) {
		if (familyIndex >= queueFamilyProperties.size())
			throw "VulkanQueuePoolLayoutInvalidFamilyIndex";
	}
	return queueFamilyProperties.at(familyIndex).maxCount;
}

bool vk::QueuePoolLayout::checkQueuePresentSupport(
  uint familyIndex, const OSRenderSurface& surface) const {
	if constexpr (DO_SAFETY_CHECK) {
		if (familyIndex >= queueFamilyProperties.size())
			throw "VulkanQueuePoolLayoutInvalidFamilyIndex";
	}
	VkBool32 support = false;
	auto result = vkGetPhysicalDeviceSurfaceSupportKHR(
	  pd, familyIndex, surface.surface, &support);
	switch (result) {
	case VK_SUCCESS: return support;
	case VK_ERROR_OUT_OF_HOST_MEMORY: throw "VulkanOutOfMemory";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY: throw "VulkanOutOfDeviceMemory";
	case VK_ERROR_SURFACE_LOST_KHR: throw "TODOVulkanSurfaceLost";
	default: throw "VulkanUnknownErrorEnum";
	}
}

void QueuePoolLayout::addQueueGroup(uint familyIndex, uint count) {
	if constexpr (DO_SAFETY_CHECK) {
		if (count > MAX_QUEUE_COUNT_PER_FAMILY)
			throw "VulkanQueuePoolLayoutAddQueueGroupQueueCountTooBig";
	}
	// FIXME: Check if total amount of queues are too much
	// (LogicalDevice.limits...)
	queueGroups.emplace_back(familyIndex, count);
}

void vk::QueuePoolLayout::hintGroupUsage(HintUsage usage, uint familyIndex) {
	switch (usage) {
	case Memory: hint.memoryIndex = familyIndex; return;
	case Present: hint.presentIndex = familyIndex; return;
	case Render: hint.renderIndex = familyIndex; return;
	default: throw "VulkanInvalidEnum";
	}
}

QueuePool::QueuePool(const QueuePoolLayout& layout, LogicalDevice& device):
  d(device) {
	groups.reserve(layout.queueGroups.size());
	for (auto& [familyIndex, queueCount] : layout.queueGroups) {
		auto& qFamProp = layout.queueFamilyProperties.at(familyIndex);
		auto& [_, group] = *groups
							  .emplace(familyIndex,
								Group{
								  .allowedUsages = qFamProp.allowedUsages,
								})
							  .first;
		group.queues.resize(queueCount);
		group.commendPools.clear();
	}
	hint = layout.hint;
}

uint QueuePool::findGroupBySupportType(
  Flags<QueueType> mustSupport, Flags<QueueType> mustNotSupport) const {
	for (auto& [i, group] : groups) {
		if (group.allowedUsages.hasAll(mustSupport)
			&& !group.allowedUsages.hasAny(mustNotSupport))
			return i;
	}
	return uint(-1);
}

uint QueuePool::getGroupByHint(HintUsage usage) const {
	switch (usage) {
	case Memory: return hint.memoryIndex;
	case Present: return hint.presentIndex;
	case Render: return hint.renderIndex;
	}
	throw "VulkanInvalidEnum";
}

CommendPool& QueuePool::queryCommendPool(
  uint groupId, Flags<CommendPoolType> type) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!groups.contains(groupId)) throw "VulkanQueuePoolInvalidGroupId";
	}
	auto& g = groups.at(groupId);
	if (&g == nullptr) throw "ererer";
	if (this == nullptr) throw "erere";

	auto pair =
	  g.commendPools.try_emplace(static_cast<uint>(type), *this, groupId, type);
	return pair.first->second;
}

Queue& vk::QueuePool::getExpressMemoryCopyQueue() {
	uint groupId = getGroupByHint(HintUsage::Memory);
	if (groupId == uint(-1))
		groupId = findGroupBySupportType(QueueType::MemoryTransfer);
	// TODO: use express queue
	Queue* q = queryQueueInUse(groupId);
	if (q == nullptr) q = queryQueueNotInUse(groupId);
	return *q;
}

Queue& vk::QueuePool::getExpressRenderQueue() {
	uint groupId = getGroupByHint(HintUsage::Render);
	if (groupId == uint(-1))
		groupId = findGroupBySupportType(QueueType::Graphics);
	// TODO: use express queue
	Queue* q = queryQueueInUse(groupId);
	if (q == nullptr) q = queryQueueNotInUse(groupId);
	return *q;
}

Queue* QueuePool::queryQueueInUse(uint groupId) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!groups.contains(groupId)) throw "VulkanQueuePoolInvalidGroupId";
	}
	auto& group = groups.at(groupId);
	for (auto& ptr : group.queues) {
		if (ptr.has_value()) return &*ptr;
	}
	return nullptr;
}

Queue* QueuePool::queryQueueNotInUse(uint groupId) {
	if constexpr (DO_SAFETY_CHECK) {
		if (!groups.contains(groupId)) throw "VulkanQueuePoolInvalidGroupId";
	}
	auto& group = groups.at(groupId);
	for (size_t queueId = 0; queueId < group.queues.size(); queueId++) {
		auto& qPtr = group.queues.at(queueId);
		if (!qPtr.has_value()) return &qPtr.make(*this, groupId, queueId);
	}
	return nullptr;
}

void vk::QueuePool::markQueueNotInUse(Queue& queue) {
	if constexpr (DO_SAFETY_CHECK) {
		if (&queue.getQueuePool() != this)
			throw "VulkanQueuePoolQueueNotChildOfQueuePool";
	}
	groups.at(queue.familyIndex).queues.at(queue.queueIndex).reset();
}

Queue::Queue(QueuePool& queuePool, uint fIndex, uint qIndex):
  qp(queuePool), familyIndex(fIndex), queueIndex(qIndex) {
	vkGetDeviceQueue(qp.getDevice().d, fIndex, qIndex, &q);
}

void vk::Queue::submit(CommendBuffer& cb, Semaphore* waitFor,
  PipelineStage waitType, Semaphore* signalToSp, Fence* signalToFc) {
	VkPipelineStageFlags wType = (VkPipelineStageFlags)waitType;
	VkSubmitInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	sInfo.waitSemaphoreCount = waitFor ? 1 : 0;
	sInfo.pWaitSemaphores = waitFor ? &waitFor->sp : nullptr;
	sInfo.pWaitDstStageMask = &wType;
	sInfo.commandBufferCount = 1;
	sInfo.pCommandBuffers = &cb.cb;
	sInfo.signalSemaphoreCount = signalToSp ? 1 : 0;
	sInfo.pSignalSemaphores = signalToSp ? &signalToSp->sp : nullptr;
	vkQueueSubmit(q, 1, &sInfo, signalToFc ? signalToFc->fc : nullptr);
}

void Queue::presentImage(
  SwapchainImage& scImg, Semaphore& presentSemaphore, Fence& cleanupFence) {
	scImg.imgAquireSpOrCompleteFc.emplace<Fence*>(&cleanupFence);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &presentSemaphore.sp;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &scImg.sc.sc;
	presentInfo.pImageIndices = &scImg.imgId;
	// FIXME: Make this allow to set queue
	auto pCode = vkQueuePresentKHR(q, &presentInfo);
	switch (pCode) {
	case VK_SUCCESS: break;
	case VK_SUBOPTIMAL_KHR: logger("Note: SubotimalKhr recieved."); break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		scImg.sc.invalidateSwapchain();
		throw OutdatedSwapchainException();
		break;
	default: throw "VKQueuePresentUnknownError";
	}
}
