#include "pch.h"

#include <memory_resource>
#include "../Planetery Engine PreBuild v0.3/Define.h"
#include "../Planetery Engine PreBuild v0.3/MemoryResource.h"

#include <stack>
#include <functional>
#include <ostream>

TEST(StackMemoryResource, BasicUsage)
{
	ASSERT_EQ(alignUpTo(12, 8), 16);
	ASSERT_EQ(alignUpTo(11, 8), 16);
	ASSERT_EQ(alignUpTo(12, 4), 12);
	ASSERT_EQ(alignUpTo(3, 8), 8);

	ASSERT_ANY_THROW(pmr::StackMemoryResource<> smr{ (size_t)0 });

	pmr::StackMemoryResource<sizeof(uint)> smr{10000};
	pmr::pmrAllocator<uint, pmr::StackMemoryResource<sizeof(uint)>> alloc(&smr);

	std::hash<uint> hasher;
	
	struct TestAlloc {
		uint value;
		size_t size;
		uint* ptr;
	};

	std::deque<TestAlloc> backStack{};

	auto allocFunc = [&](size_t c) {
		for (size_t i = 0; i < c; i++) {
			TestAlloc& p = backStack.emplace_back();
			p.value = i;
			p.size = hasher(i) % 10;
			p.ptr = alloc.allocate_object<uint>(p.size);
			std::fill_n(p.ptr, p.size, p.value);
		}
	};
	auto deallocFunc = [&](size_t c) {
		for (size_t i = 0; i < c; i++) {
			TestAlloc& p = backStack.back();
			for (uint offset = 0; offset < p.size; offset++) {
				ASSERT_EQ(p.ptr[offset], p.value);
			}
			alloc.deallocate_object<uint>(p.ptr, p.size);
			backStack.pop_back();
		}
	};
	auto checkFunc = [&]() {
		for (auto& p : backStack) {
			for (uint offset = 0; offset < p.size; offset++) {
				ASSERT_EQ(p.ptr[offset], p.value);
			}
		}
	};

	allocFunc(10000);
	checkFunc();
	deallocFunc(10000);
	while (backStack.size() < 10000)
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
	allocFunc(5000);
	smr.release();
	ASSERT_DEATH(deallocFunc(1), "");

}


