//Design issue:
//Major Bug #3 unfixable without heavy preformance hit.


#include <compare>
#include <forward_list>
#include <map>
#include <assert.h>
#include <deque>
#include <stdlib.h>
#include <list>

using VkDeviceMemory = void*;
using uint = unsigned int;

class LogicalDevice {
public:
	LogicalDevice() {}
	LogicalDevice(const LogicalDevice&) = delete;
	LogicalDevice(LogicalDevice&&) = default;
	size_t id = 0;
};
enum class MemoryFeature {
	A,
	B,
	C,
	D,
};


// Memory Class
namespace vk {
	struct DeviceMemory
	{
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
			size_t ext = 0;

			bool free = true;

		};
		struct Group {
			Group(size_t size, void* ptrDebug);
			char* ptrDebug;
			// key as DeviceMemory
			std::map<size_t, State> nodes;
			// Note that since std::set iters are not effected by other
			// insert/erase, I can store a dqueue of freeNotes to get faster
			// alloc.
			std::forward_list<std::map<size_t, State>::iterator> freeNodes;
			size_t alloc(size_t n, size_t align);  // on Fail return -1
			void free(size_t offset);
		};
		std::map<DeviceMemory, Group> groups;
		size_t blockSize;
		MemoryAllocator& upperAllocator;
	public:
		MemoryPool(size_t initialSize, MemoryAllocator targetAllocator);
		MemoryPointer alloc(size_t n, size_t align);
		void free(MemoryPointer ptr);
	};
}

using namespace vk;

// HOTFIX: for error C2572 redef of default argument on 'std::next'
template<class T> inline auto Next(T iter) { return ++iter; }

// HOTFIX: for error C2572 redef of default argument on 'std::prev'
template<class T> inline auto Prev(T iter) { return --iter; }



MemoryAllocator::MemoryAllocator(
	LogicalDevice& d, MemoryFeature feature, uint memoryIndex) :
	d(d) {
	this->feature = feature;
	this->memoryIndex = memoryIndex;
}
DeviceMemory MemoryAllocator::alloc(size_t n) {
	DeviceMemory dm{};
	dm.dm = std::malloc(n);
	return dm;
}
void MemoryAllocator::free(DeviceMemory m) {
	std::free(m.dm);
}

vk::MemoryPool::Group::Group(size_t size, void* ptr) {
	freeNodes.emplace_front(nodes.emplace(0, State{}).first); //FIX NO 1: Forgot the freeNotes adding
	nodes.emplace(size, State{});
	//DEBUG
	ptrDebug = (char*)ptr;
	std::fill_n(ptrDebug, size, 'f');
	*ptrDebug = 'F';
}

//MAJOR BUG 1: Waste tons of space on big align requirement
//MAJOR BUG 2: if ext>n, it splicts chucks and cause mis ordering
//MAJOR BUG 3: On free and merging 3 blocks, it did not remove the first item from freeNodes
size_t MemoryPool::Group::alloc(size_t n, size_t align) {
	auto iter = freeNodes.before_begin();
	auto iterNext = Next(iter);
	while (iterNext != freeNodes.end()) {
		auto& [offset, state] = **iterNext;
		auto& [nOffset, nState] = *Next(*iterNext);
		size_t result = (offset - state.ext);
		if ((result & (align - 1)) != 0)
			result = (result & ~(align - 1)) + align;
		size_t nextPoint = result + n;
		if (nOffset - nState.ext < nextPoint) {
			iter = iterNext;
			iterNext = Next(iter); //FIX NO 2: Missing line
			continue;
		}
		state.free = false;
		if (nOffset - nState.ext <= nextPoint + MIN_NODE_SIZE) {
			//DEBUG
			std::fill(ptrDebug + offset - state.ext, ptrDebug + nOffset - nState.ext , 'w');
			std::fill(ptrDebug + result, ptrDebug + nextPoint, 'u');
			*(ptrDebug + offset - state.ext) = *(ptrDebug + offset - state.ext) == 'w' ? 'W' : 'U';

			freeNodes.erase_after(iter);
			return result; //Fix NO 3: offset -> result
		}
		if (nextPoint <= offset) { //MAJOR FIX BUG #2: added this check for fixing the ext>n problem
			//DEBUG
			std::fill(ptrDebug + offset - state.ext, ptrDebug + nextPoint, 'w');
			std::fill(ptrDebug + result, ptrDebug + nextPoint, 'u');
			*(ptrDebug + offset - state.ext) = *(ptrDebug + offset - state.ext) == 'w' ? 'W' : 'U';
			*(ptrDebug + nextPoint) = 'F';

			auto v = nodes.emplace_hint(*iterNext, offset - state.ext, State{});
			v->second.free = false;
			state.ext = offset - nextPoint;
			state.free = true;
			return result;
		}


		//DEBUG
		std::fill(ptrDebug + offset - state.ext, ptrDebug + nextPoint, 'w');
		std::fill(ptrDebug + result, ptrDebug + nextPoint, 'u');
		*(ptrDebug + offset - state.ext) = *(ptrDebug + offset - state.ext) == 'w' ? 'W' : 'U';
		*(ptrDebug + nextPoint) = 'F';

		*iterNext = nodes.emplace_hint(Next(*iterNext), nextPoint, State{}); //FIX NO 5: added missing '*iterNext = '
		return result; //Fix NO 3: offset -> result
	}
	return size_t(-1);
}
/* --OLD VERSION: Needs complete restructure
void MemoryPool::Group::free(size_t ptr) {
	auto iter = nodes.lower_bound(ptr);
	if (iter == nodes.end()) throw "VulkanMemoryPoolCorruptionError";
	if (iter->first - iter->second.ext > ptr) {
		if (iter == nodes.begin()) throw "VulkanMemoryPoolCorruptionError";
		--iter;
	}
	else if (iter == Prev(nodes.end()))
		throw "VulkanMemoryPoolCorruptionError";
	auto preIter = Prev(iter);
	auto nextIter = Next(iter);
	bool preIterFree = iter != nodes.begin() && preIter->second.free;
	bool nextIterFree = nextIter->second.free;
	if (!preIterFree && !nextIterFree) {
		//DEBUG
		std::fill(ptrDebug + iter->first - iter->second.ext, ptrDebug + nextIter->first - nextIter->second.ext, 'f');
		*(ptrDebug + iter->first - iter->second.ext) = 'F';

		iter->second.free = true;
		freeNodes.push_front(iter);
		return;
	}
	if (preIterFree) {
		//DEBUG
		std::fill(ptrDebug + iter->first - iter->second.ext, ptrDebug + nextIter->first - nextIter->second.ext, 'f');

		// As no length is stored, it sould work
		nodes.erase(iter);
		iter = preIter;
	}
	else {
		//DEBUG
		std::fill(ptrDebug + iter->first - iter->second.ext, ptrDebug + nextIter->first - nextIter->second.ext, 'f');
		*(ptrDebug + iter->first - iter->second.ext) = 'F';
	}

	if (nextIterFree) {
		//DEBUG
		*(ptrDebug + nextIter->first - nextIter->second.ext) = 'f';

		nextIter->second.ext = nextIter->first - iter->first + iter->second.ext;
		
		nodes.erase(iter);
	}
}*/
void MemoryPool::Group::free(size_t ptr) {
	auto iter = nodes.lower_bound(ptr);
	if (iter == nodes.end()) throw "VulkanMemoryPoolCorruptionError";
	if (iter->first - iter->second.ext > ptr) {
		if (iter == nodes.begin()) throw "VulkanMemoryPoolCorruptionError";
		--iter;
	}
	else if (iter == Prev(nodes.end()))
		throw "VulkanMemoryPoolCorruptionError";
	auto preIter = Prev(iter);
	auto nextIter = Next(iter);
	bool preIterFree = iter != nodes.begin() && preIter->second.free;
	bool nextIterFree = Next(nextIter) != nodes.end() && nextIter->second.free; //FIX NO 6: added missing 'Next(nextIter) != nodes.end() &&'

	if (preIterFree) { //Merge with previous node
		//DEBUG
		std::fill(ptrDebug + iter->first - iter->second.ext, ptrDebug + nextIter->first - nextIter->second.ext, 'f');

		// As no length is stored, it sould work
		nodes.erase(iter);
	}
	else if (nextIterFree) { //Merge with next node
		//DEBUG
		std::fill(ptrDebug + iter->first - iter->second.ext, ptrDebug + nextIter->first - nextIter->second.ext + 1, 'f');
		*(ptrDebug + iter->first - iter->second.ext) = 'F';

		nextIter->second.ext = nextIter->first - iter->first + iter->second.ext;
		nodes.erase(iter);
	}
	else {
		//DEBUG
		std::fill(ptrDebug + iter->first - iter->second.ext, ptrDebug + nextIter->first - nextIter->second.ext, 'f');
		*(ptrDebug + iter->first - iter->second.ext) = 'F';

		iter->second.free = true;
		freeNodes.push_front(iter);
	}
}

MemoryPool::MemoryPool(size_t initialSize, MemoryAllocator targetAllocator) : upperAllocator(targetAllocator) {
	blockSize = initialSize;
	auto p = targetAllocator.alloc(initialSize);
	groups.emplace(p, Group(initialSize, p.dm));
}

MemoryPointer MemoryPool::alloc(size_t n, size_t align) {
	if (n > blockSize) throw "TOFIX:VulkanMemoryPoolAllocSizeGreaterThenBlockSize";
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

	auto& [dm, group] =
		*(groups.emplace(_p, Group(blockSize,_p.dm)).first);
	mp.dm = dm;
	mp.offset = group.alloc(n, align);
	assert(mp.offset != size_t(-1));
	return mp;
}

void MemoryPool::free(MemoryPointer ptr) {
	groups.at(ptr.dm).free(ptr.offset);
}

size_t genAlign(size_t i) {
	i = i % 12;
	switch (i)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return 1;
	case 4:
	case 5:
	case 6:
		return 2;
	case 7:
	case 8:
		return 4;
	case 9:
		return 8;
	case 10:
		return 32;
	case 11:
		return 128;
	}
	return 1;
}


char* asPtr(MemoryPointer ptr) { return (char*)(ptr.dm.dm)+ptr.offset; }
char* asPtr(void* ptr, size_t offset) { return (char*)(ptr)+offset; }

template <class T>
auto getAt(std::list<T>& t, size_t i) {
	auto iter = t.begin();
	return std::next(iter, i);
}




int main() {
	srand(10);



	LogicalDevice d{};
	MemoryAllocator mAllc{ d,MemoryFeature::A,0 };
	MemoryPool mPool{ 2048,mAllc };

	std::hash<uint> hasher;
	struct TestAlloc {
		char value;
		size_t size; //bytesize
		size_t align;
		MemoryPointer ptr;
	};
	std::list<TestAlloc> backStack{};
	auto allocFunc = [&](size_t c) {
		for (size_t i = 0; i < c; i++) {
			TestAlloc& p = *backStack.emplace(getAt(backStack, rand() % (backStack.size()+1)));
			p.value = (char)rand();
			p.size = rand() % 256 + 1;
			p.align = genAlign(rand());
			p.ptr = mPool.alloc(p.size, p.align);
			assert(p.ptr.offset%p.align == 0);
			std::fill(asPtr(p.ptr), asPtr(p.ptr)+p.size, p.value);
		}
	};
	auto deallocFunc = [&](size_t c) {
		for (size_t i = 0; i < c; i++) {
			auto iter = getAt(backStack, rand() % backStack.size());
			TestAlloc& p = *iter;
			for (size_t offset = 0; offset < p.size; offset++) {
				assert(*(asPtr(p.ptr) + offset) == p.value);
			}
			mPool.free(p.ptr);
			backStack.erase(iter);
		}
	};
	auto checkFunc = [&]() {
		for (auto& p : backStack) {
			for (size_t offset = 0; offset < p.size; offset++) {
				assert(*(asPtr(p.ptr) + offset) == p.value);
			}
		}
	};





	allocFunc(1000);
	checkFunc();
	deallocFunc(1000);
	while (backStack.size() < 1000)
	{
		allocFunc(4);
		deallocFunc(3);
		checkFunc();
	}
	while (backStack.size() > 4)
	{
		deallocFunc(4);
		allocFunc(3);
		checkFunc();
	}
	deallocFunc(backStack.size());
}