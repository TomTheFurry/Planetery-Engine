module Vulkan: MemoryImp;
import: Memory;
import: Enum;
import: Device;
import "VulkanExtModule.h";

using namespace vk;


// HOTFIX: for error C2572 redef of default argument on 'std::next'
template<class T> inline auto Next(T iter) { return ++iter; }

// HOTFIX: for error C2572 redef of default argument on 'std::prev'
template<class T> inline auto Prev(T iter) { return --iter; }


MemoryAllocator::MemoryAllocator(
  LogicalDevice& d, MemoryFeature feature, uint memoryIndex):
  d(d) {
	this->feature = feature;
	this->memoryIndex = memoryIndex;
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
	return dm;
}
void MemoryAllocator::free(DeviceMemory m) { vkFreeMemory(d.d, m.dm, nullptr); }


MemoryPool::Group::Group(size_t size, void* ptr) {
	auto iter = nodes.emplace(0, State{}).first;
	iter->second.isFree = freeNodes.emplace(0, iter).first;
	nodes.emplace(size, State{freeNodes.end()});

	// DEBUG
	ptrDebug = (char*)ptr;
	std::fill(ptrDebug, ptrDebug + size, 'f');
	*ptrDebug = 'F';
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

		// DEBUG
		std::fill(ptrDebug + offset, ptrDebug + nOffset, 'w');
		std::fill(ptrDebug + result, ptrDebug + nextPoint, 'u');
		*(ptrDebug + offset) = *(ptrDebug + offset) == 'w' ? 'W' : 'U';

		if (nOffset - nextPoint >= MIN_NODE_SIZE) {
			// DEBUG
			std::fill(ptrDebug + nextPoint, ptrDebug + nOffset, 'f');
			*(ptrDebug + nextPoint) = 'F';

			if (nState.isFree != freeNodes.end()) {
				// DEBUG
				assert(nState.isFree == Next(freeIter));
				*(ptrDebug + nOffset) = 'f';

				nodes.erase(Next(freeIter->second));
				freeNodes.erase(Next(freeIter));
			}

			auto iter =
			  nodes.emplace_hint(Next(freeIter->second), nextPoint, State{});
			iter->second.isFree =
			  freeNodes.emplace_hint(Next(freeIter), nextPoint, iter);
		}
		if (result - offset >= MIN_NODE_SIZE) {
			// DEBUG
			std::fill(ptrDebug + offset, ptrDebug + result, 'f');
			*(ptrDebug + offset) = 'F';

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
		// DEBUG
		*(ptrDebug + Next(iter)->first) = 'f';

		freeNodes.erase(Next(iter)->second.isFree);
		nodes.erase(Next(iter));
	}
	// DEBUG
	std::fill(ptrDebug + iter->first, ptrDebug + Next(iter)->first, 'f');
	if (prevFree) {
		// No need for updating anything as I just expend previous node by
		// deleting current node
		nodes.erase(iter);
	} else {
		// DEBUG
		*(ptrDebug + iter->first) = 'F';

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
MemoryPool::~MemoryPool() noexcept(false) {
	if (!groups.empty()) throw "VulkanMemoryPoolMissingFree";
}

MemoryPointer MemoryPool::alloc(size_t n, size_t align) {
	if (n > blockSize)
		throw "TOFIX:VulkanMemoryPoolAllocSizeGreaterThenBlockSize";
	MemoryPointer mp{};
	for (auto& [dm, group] : groups) {
		size_t offset = group.alloc(n, align);
		if (offset != size_t(-1)) {
			mp.offset = offset;
			mp.dm = dm;
			return mp;
		}
	}
	auto _p = upperAllocator.alloc(blockSize);

	auto& [dm, group] = *(groups.emplace(_p, Group(blockSize, _p.dm)).first);
	mp.dm = dm;
	mp.offset = group.alloc(n, align);
	assert(mp.offset != size_t(-1));
	return mp;
}

void MemoryPool::free(MemoryPointer ptr) {
	groups.at(ptr.dm).free(ptr.offset);
	if (groups.at(ptr.dm).isEmpty()) {
		groups.erase(ptr.dm);
		upperAllocator.free(ptr.dm);
	}
}