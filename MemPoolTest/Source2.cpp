
#include <compare>
#include <forward_list>
#include <map>
#include <assert.h>
#include <deque>
#include <stdlib.h>
#include <list>
#include <set>
#include <iostream>

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
			std::map<size_t, std::map<size_t, State>::iterator>::iterator isFree;
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
	std::cout << "AllocCall: " << n << ", ";
	return dm;
}
void MemoryAllocator::free(DeviceMemory m) {
	std::free(m.dm);
	std::cout << "FreeCall, ";
}

vk::MemoryPool::Group::Group(size_t size, void* ptr) {
	auto iter = nodes.emplace(0, State{}).first;
	iter->second.isFree = freeNodes.emplace(0, iter).first;
	nodes.emplace(size, State{ freeNodes.end() });

	//DEBUG
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

		//DEBUG
		std::fill(ptrDebug + offset, ptrDebug + nOffset, 'w');
		std::fill(ptrDebug + result, ptrDebug + nextPoint, 'u');
		*(ptrDebug + offset) = *(ptrDebug + offset) == 'w' ? 'W' : 'U';

		if (nOffset - nextPoint >= MIN_NODE_SIZE) {
			//DEBUG
			std::fill(ptrDebug + nextPoint, ptrDebug + nOffset, 'f');
			*(ptrDebug + nextPoint) = 'F';

			if (nState.isFree != freeNodes.end()) {
				//DEBUG
				assert(nState.isFree == Next(freeIter));
				*(ptrDebug + nOffset) = 'f';

				nodes.erase(Next(freeIter->second));
				freeNodes.erase(Next(freeIter));
			}

			auto iter = nodes.emplace_hint(Next(freeIter->second), nextPoint, State{});
			iter->second.isFree = freeNodes.emplace_hint(Next(freeIter), nextPoint, iter);
		}
		if (result - offset >= MIN_NODE_SIZE) {
			//DEBUG
			std::fill(ptrDebug + offset, ptrDebug + result, 'f');
			*(ptrDebug + offset) = 'F';

			//No need for updating anything as I just shotten current node by making a next node
			nodes.emplace_hint(Next(freeIter->second), result, State{ freeNodes.end() });
		} else {
			//Remove current node from freeNodes
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
	if (iter->second.isFree != freeNodes.end()) throw "VulkanMemoryPoolCorruption";

	auto& offset = iter->first;
	bool prevFree = iter != nodes.begin() && Prev(iter)->second.isFree != freeNodes.end();
	bool nextFree = Next(iter)->second.isFree != freeNodes.end();
	
	if (nextFree) {
		//DEBUG
		*(ptrDebug + Next(iter)->first) = 'f';

		freeNodes.erase(Next(iter)->second.isFree);
		nodes.erase(Next(iter));
	}
	//DEBUG
	std::fill(ptrDebug + iter->first, ptrDebug + Next(iter)->first, 'f');
	if (prevFree) {
		//No need for updating anything as I just expend previous node by deleting current node
		nodes.erase(iter);
	}
	else {
		//DEBUG
		*(ptrDebug + iter->first) = 'F';

		iter->second.isFree = freeNodes.emplace(iter->first, iter).first; //Slow... Design flaw :(
	}
}
bool MemoryPool::Group::isEmpty() const
{
	return nodes.size() == 2 && freeNodes.size() == 1;
}

MemoryPool::MemoryPool(size_t initialSize, MemoryAllocator targetAllocator) : upperAllocator(targetAllocator) {
	blockSize = initialSize;
}
MemoryPool::~MemoryPool() noexcept(false)
{
	if (!groups.empty()) throw "VulkanMemoryPoolMissingFree";
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
		*(groups.emplace(_p, Group(blockSize, _p.dm)).first);
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


char* asPtr(MemoryPointer ptr) { return (char*)(ptr.dm.dm) + ptr.offset; }
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
	MemoryPool mPool{ 2048*16,mAllc };

	struct TestAlloc {
		char value;
		size_t size; //bytesize
		size_t align;
		MemoryPointer ptr;
	};
	std::list<TestAlloc> backStack{};
	auto checkFunc = [&]() {
		for (auto& p : backStack) {
			for (size_t offset = 0; offset < p.size; offset++) {
				assert(*(asPtr(p.ptr) + offset) == p.value);
			}
		}
	};
	auto allocFunc = [&](size_t c) {
		for (size_t i = 0; i < c; i++) {
			TestAlloc& p = *backStack.emplace(getAt(backStack, rand() % (backStack.size() + 1)));
			p.value = (char)rand();
			p.size = rand() % 256 + 1;
			p.align = genAlign(rand());
			p.ptr = mPool.alloc(p.size, p.align);
			assert(p.ptr.offset % p.align == 0);
			std::fill(asPtr(p.ptr), asPtr(p.ptr) + p.size, p.value);
			checkFunc();
			//std::cout << p.size << '\n';
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