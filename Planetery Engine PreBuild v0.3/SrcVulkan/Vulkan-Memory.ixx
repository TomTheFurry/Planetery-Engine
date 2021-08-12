export module Vulkan: Memory;
export import: Internal;
import: Enum;
import std.core;
import Define;
import Util;
import "VulkanExtModule.h";

// Memory Class
namespace vk {
	struct DeviceMemory {
		VkDeviceMemory dm;
		auto operator<=>(const DeviceMemory&) const = default;
	};
	struct MemoryPointer {
		DeviceMemory dm;
		size_t offset;
	};

	class MemoryAllocator
	{
	  public:
		MemoryAllocator(
		  LogicalDevice& d, MemoryFeature feature, uint memoryIndex);
		MemoryAllocator(const MemoryAllocator&) = default;
		MemoryAllocator(MemoryAllocator&&) = default;
		DeviceMemory alloc(size_t n);
		void free(DeviceMemory m);
		MemoryFeature feature;
		uint memoryIndex;
		LogicalDevice& d;
	};

	class MemoryPool
	{
		static constexpr size_t MIN_NODE_SIZE = sizeof(float) * 2;
		struct State {
			std::map<size_t, std::map<size_t, State>::iterator>::iterator
			  isFree;
		};
		struct Group {
			Group(size_t size, void* ptrDebug);
			char* ptrDebug;
			// key as DeviceMemory
			std::map<size_t, State> nodes;
			std::map<size_t, std::map<size_t, State>::iterator> freeNodes;
			size_t alloc(size_t n, size_t align);  // on Fail return -1
			void free(size_t offset);
			bool isEmpty() const;
		};
		std::map<DeviceMemory, Group> groups;
		size_t blockSize;
		MemoryAllocator& upperAllocator;

	  public:
		MemoryPool(size_t initialSize, MemoryAllocator targetAllocator);
		MemoryPointer alloc(size_t n, size_t align);
		void free(MemoryPointer ptr);
		~MemoryPool() noexcept(false);
	};
}