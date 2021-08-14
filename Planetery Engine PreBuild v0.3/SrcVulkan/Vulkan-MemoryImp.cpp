module Vulkan: MemoryImp;
import: Memory;
import: Enum;
import: Device;
import Logger;
import Util;
import "VulkanExtModule.h";

using namespace vk;


// HOTFIX: for error C2572 redef of default argument on 'std::next'
template<class T> inline auto Next(T iter) { return ++iter; }

// HOTFIX: for error C2572 redef of default argument on 'std::prev'
template<class T> inline auto Prev(T iter) { return --iter; }


MemoryAllocator::MemoryAllocator(
  LogicalDevice& d, uint memoryIndex):
  d(d) {
	this->memoryIndex = memoryIndex;
	logger("VulkanMemory(", memoryIndex, "): Init\n");
}
DeviceMemory MemoryAllocator::alloc(size_t n) {
	VkMemoryAllocateInfo allocInfo{
	  .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	  .pNext = nullptr,
	  .allocationSize = n,
	  .memoryTypeIndex = memoryIndex,
	};
	DeviceMemory dm{};
	vkAllocateMemory(d.d, &allocInfo, nullptr, &dm.dm);
	logger(
	  "VulkanMemory(", memoryIndex, "): Alloc ", byte(n), " at ", address(dm.dm), "\n");
	return dm;
}
void MemoryAllocator::free(DeviceMemory m) {
	logger("VulkanMemory(", memoryIndex, "): Free ",
	  address(m.dm), "\n");
	vkFreeMemory(d.d, m.dm, nullptr);
}


MemoryPool::Group::Group(size_t size) {
	auto iter = nodes.emplace(0, State{}).first;
	iter->second.isFree = freeNodes.emplace(0, iter).first;
	nodes.emplace(size, State{freeNodes.end()});
}

size_t MemoryPool::Group::alloc(size_t n, size_t align) {
	auto freeIter = freeNodes.begin();
	while (freeIter != freeNodes.end()) {
		auto& [offset, state] = *(freeIter->second);
		auto& [nOffset, nState] = *Next(freeIter->second);
		if (nOffset - offset < n) {
			++freeIter;
			continue;
		}
		size_t result = offset;
		if ((result & (align - 1)) != 0)
			result = (result & ~(align - 1)) + align;
		size_t nextPoint = result + n;
		if (nOffset < nextPoint) {
			++freeIter;
			continue;
		}

		if (nOffset - nextPoint >= MIN_NODE_SIZE) {

			if (nState.isFree != freeNodes.end()) {

				nodes.erase(Next(freeIter->second));
				freeNodes.erase(Next(freeIter));
			}

			auto iter =
			  nodes.emplace_hint(Next(freeIter->second), nextPoint, State{});
			iter->second.isFree =
			  freeNodes.emplace_hint(Next(freeIter), nextPoint, iter);
		}
		if (result - offset >= MIN_NODE_SIZE) {
			// No need for updating anything as I just shotten current node by
			// making a next node
			nodes.emplace_hint(
			  Next(freeIter->second), result, State{freeNodes.end()});
		} else {
			// Remove current node from freeNodes
			freeNodes.erase(freeIter);
			state.isFree = freeNodes.end();
		}
		return result;
	}
	return size_t(-1);
}

void MemoryPool::Group::free(size_t ptr) {
	auto iter = nodes.lower_bound(ptr);

	if (iter->first != ptr) --iter;
	if (Next(iter) == nodes.end()) throw "VulkanMemoryPoolCorruption";
	if (iter->second.isFree != freeNodes.end())
		throw "VulkanMemoryPoolCorruption";

	auto& offset = iter->first;
	bool prevFree =
	  iter != nodes.begin() && Prev(iter)->second.isFree != freeNodes.end();
	bool nextFree = Next(iter)->second.isFree != freeNodes.end();

	if (nextFree) {

		freeNodes.erase(Next(iter)->second.isFree);
		nodes.erase(Next(iter));
	}
	if (prevFree) {
		// No need for updating anything as I just expend previous node by
		// deleting current node
		nodes.erase(iter);
	} else {

		iter->second.isFree =
		  freeNodes.emplace(iter->first, iter).first;  // Slow... Design flaw :(
	}
}
bool MemoryPool::Group::isEmpty() const {
	return nodes.size() == 2 && freeNodes.size() == 1;
}

MemoryPool::MemoryPool(size_t initialSize, MemoryAllocator targetAllocator):
  upperAllocator(targetAllocator) {
	blockSize = initialSize;
}
MemoryPool::MemoryPool(MemoryPool&& o):
  groups(std::move(o.groups)), upperAllocator(o.upperAllocator),
  blockSize(o.blockSize) {}

MemoryPool::~MemoryPool() noexcept(false) {
	if (!groups.empty()) throw "VulkanMemoryPoolMissingFree";
}

MemoryPointer MemoryPool::alloc(size_t n, size_t align) {
	MemoryPointer mp{};
	if (n > blockSize) {
		mp.dm = upperAllocator.alloc(n);
		mp.offset = 0;
		return mp;
	}
	for (auto& [dm, group] : groups) {
		size_t offset = group.alloc(n, align);
		if (offset != size_t(-1)) {
			mp.offset = offset;
			mp.dm = dm;
			return mp;
		}
	}
	auto _p = upperAllocator.alloc(blockSize);
	auto& [dm, group] = *(groups.emplace(_p, Group(blockSize)).first);
	mp.dm = dm;
	mp.offset = group.alloc(n, align);
	if (mp.offset == size_t(-1)) throw "VULKANASSERTFAILURE";
	return mp;
}

void MemoryPool::free(MemoryPointer ptr) {
	auto iter = groups.find(ptr.dm);
	if (ptr.offset == 0) {
		if (iter == groups.end()) {
			upperAllocator.free(ptr.dm);
			return;
		}
	}
	if (iter == groups.end()) throw "VULKANASSERTFAILURE";

	iter->second.free(ptr.offset);
	if (iter->second.isEmpty()) {
		groups.erase(iter);
		upperAllocator.free(ptr.dm);
	}
}