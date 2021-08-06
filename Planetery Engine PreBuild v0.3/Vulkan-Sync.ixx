export module Vulkan: Sync;
export import: Internal;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Sync class:
export namespace vk {
	class Fence
	{
	  public:
		Fence(vk::LogicalDevice& d, bool signaled = false);
		Fence(const Fence&) = delete;
		Fence(Fence&& o) noexcept;
		~Fence();
		void wait();
		VkFence fc;
		vk::LogicalDevice& d;
	};
	class Semaphore
	{
	  public:
		Semaphore(vk::LogicalDevice& d, bool isSet = false);
		Semaphore(const Semaphore&) = delete;
		Semaphore(Semaphore&& o) noexcept;
		~Semaphore();
		VkSemaphore sp;
		vk::LogicalDevice& d;
	};
	class TimelineSemaphore
	{
	  public:
		TimelineSemaphore(vk::LogicalDevice& d, ulint initValue);
		TimelineSemaphore(const TimelineSemaphore&) = delete;
		TimelineSemaphore(TimelineSemaphore&& o) noexcept;
		~TimelineSemaphore();
		VkSemaphore sp;
		vk::LogicalDevice& d;
	};
	struct SyncPoint {
		SyncNumber syncNumber;
		SyncLine* syncLine;
		SyncPoint next();
	};
	struct SyncLine {
		TimelineSemaphore sp;
		SyncNumber topNumber;
		SyncPoint next();
		SyncPoint top();
	};
	inline SyncPoint SyncPoint::next() { return syncLine->next(); }
	inline SyncPoint SyncLine::next() { return SyncPoint{++topNumber, this}; }
	inline SyncPoint SyncLine::top() { return SyncPoint(topNumber, this); }
}
