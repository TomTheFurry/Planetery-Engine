export module Vulkan: Sync;
export import: Declaration;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Sync class:
export namespace vk {
	class Fence: public ComplexObject
	{
	  public:
		Fence(vk::LogicalDevice& d, bool signaled = false);
		Fence(const Fence&) = delete;
		Fence(Fence&& o) noexcept;
		Fence& operator=(
		  Fence&&);	 // FIXME: Does not set the new LogicalDevice Ref
		~Fence();
		void wait();
		void reset();
		VkFence fc;
		vk::LogicalDevice& d;
	};
	class Semaphore: public ComplexObject
	{
	  public:
		Semaphore(vk::LogicalDevice& d);
		Semaphore(const Semaphore&) = delete;
		Semaphore(Semaphore&& o) noexcept;
		Semaphore& operator=(
		  Semaphore&&);	 // FIXME: Does not set the new LogicalDevice Ref
		~Semaphore();
		void reset();
		VkSemaphore sp;
		vk::LogicalDevice& d;
	};
	class TimelineSemaphore: public ComplexObject
	{
	  public:
		TimelineSemaphore(vk::LogicalDevice& d, ulint initValue);
		TimelineSemaphore(const TimelineSemaphore&) = delete;
		TimelineSemaphore(TimelineSemaphore&& o) noexcept;
		TimelineSemaphore& operator=(
		  TimelineSemaphore&&);	 // FIXME: Does not set the new LogicalDevice
								 // Ref
		~TimelineSemaphore();
		void reset(ulint initValue);
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
