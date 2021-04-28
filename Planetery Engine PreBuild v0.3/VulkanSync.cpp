module;
#include "Marco.h"
#ifdef USE_VULKAN
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

TimelineSemaphore::TimelineSemaphore(LogicalDevice& device, ulint initValue):
  d(device) {
	VkSemaphoreTypeCreateInfo stInfo{};
	stInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	stInfo.initialValue = initValue;
	stInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	VkSemaphoreCreateInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sInfo.pNext = &stInfo;
	vkCreateSemaphore(d.d, &sInfo, nullptr, &sp);
}
TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& o) noexcept: d(o.d) {
	sp = o.sp;
	o.sp = nullptr;
}
TimelineSemaphore::~TimelineSemaphore() {
	if (sp != nullptr) vkDestroySemaphore(d.d, sp, nullptr);
}

Semaphore::Semaphore(LogicalDevice& device, bool isSet): d(device) {
	VkSemaphoreCreateInfo sInfo{};
	sInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	sInfo.pNext = NULL;
	vkCreateSemaphore(d.d, &sInfo, nullptr, &sp);
}
Semaphore::Semaphore(Semaphore&& o) noexcept: d(o.d) {
	sp = o.sp;
	o.sp = nullptr;
}
Semaphore::~Semaphore() {
	if (sp != nullptr) vkDestroySemaphore(d.d, sp, nullptr);
}

Fence::Fence(LogicalDevice& device, bool signaled): d(device) {
	VkFenceCreateInfo fInfo{};
	fInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	vkCreateFence(d.d, &fInfo, nullptr, &fc);
}
Fence::Fence(Fence&& o) noexcept: d(o.d) {
	fc = o.fc;
	o.fc = nullptr;
}
Fence::~Fence() {
	if (fc != nullptr) vkDestroyFence(d.d, fc, nullptr);
}
#else
module Vulkan;
#endif