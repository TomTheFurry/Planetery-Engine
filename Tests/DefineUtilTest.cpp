#include "pch.h"

#include <memory_resource>
#include "../Planetery Engine PreBuild v0.3/Define.h"
#include "../Planetery Engine PreBuild v0.3/MemoryResource.h"

#include <stack>
#include <functional>
#include <ostream>
#include <exception>

TEST(StackMemoryResource, BasicUsage)
{
	std::hash<uint> hasher;
	pmr::StackMemoryResource<sizeof(uint)> smr{ 10000, pmr::get_default_resource() };
	pmr::pmrAllocator<uint, pmr::StackMemoryResource<sizeof(uint)>> alloc(&smr);
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
TEST(StackMemoryResource, SafeInvalidUsage)
{
	EXPECT_THROW(pmr::StackMemoryResource<> smr{ (size_t)0 }, pmr::BadArgException);
	{
		pmr::StackMemoryResource<8> smr{ 1000 };
		void* p;
		ASSERT_NO_THROW(p = smr.allocate(12));
		EXPECT_THROW(smr.allocate(-1), pmr::BadAllocException);
		EXPECT_THROW(smr.allocate(1, 16), pmr::BadAllocException);
		ASSERT_NO_FATAL_FAILURE(smr.deallocate(p, 12));
	}
	{
		EXPECT_THROW(pmr::StackMemoryResource<8> smrBad{ pmr::null_memory_resource() }, std::bad_alloc);
		auto mem = new char[1000];
		auto lrm = pmr::MonotonicResource(mem, 1000, pmr::null_memory_resource());
		pmr::StackMemoryResource<8> smrGood(100,&lrm);
		void* p;
		ASSERT_THROW(while (true) p = (char*)smrGood.allocate(10), std::bad_alloc);
		EXPECT_NO_THROW(smrGood.deallocate(p, 10));
		EXPECT_NO_THROW(p=smrGood.allocate(10));
		ASSERT_THROW(smrGood.allocate(10), std::bad_alloc);
		EXPECT_NO_THROW(smrGood.release());
		EXPECT_NO_THROW(smrGood.allocate(19));
	}
}

