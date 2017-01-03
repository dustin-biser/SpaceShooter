//
// Test_Memory.cpp
//

#include <gtest/gtest.h>

#include "Engine/Source/Core/Memory.hpp"


class MemoryGlobalsTest : public ::testing::Test {
protected:
	LinearAllocator * linearAllocator;

	void SetUp () override
	{
		memory_globals::init ();
		linearAllocator = &memory_globals::linearAllocator ();
	}

	void TearDown() override 
	{
		// Asserts no memory leaks.
		memory_globals::shutdown ();
		linearAllocator = nullptr;
	}

};


namespace
{
	struct alignas(64) SomeStruct64
	{
		int x;
	};

	struct alignas(128) SomeStruct128
	{
		int x;
	};
}

//---------------------------------------------------------------------------------------
// MemoryGlobals Tests
//---------------------------------------------------------------------------------------
TEST_F (MemoryGlobalsTest, init_is_idempotent)
{
	memory_globals::init ();
	EXPECT_EQ (0, linearAllocator->totalAllocated ());
}

TEST_F (MemoryGlobalsTest, LinearAllocator_totalAllocated)
{
	size_t expectedAllocatedTotal = 0;

	make_new (*linearAllocator, char);
	expectedAllocatedTotal += sizeof (char);

	// '<=' necessary due to memory alignment during allocation
	EXPECT_TRUE (expectedAllocatedTotal <= linearAllocator->totalAllocated ());

	make_new (*linearAllocator, SomeStruct64);
	expectedAllocatedTotal += sizeof (SomeStruct64);
	EXPECT_TRUE (expectedAllocatedTotal <= linearAllocator->totalAllocated ());

	make_new (*linearAllocator, SomeStruct128);
	expectedAllocatedTotal += sizeof (double);
	EXPECT_TRUE (expectedAllocatedTotal <= linearAllocator->totalAllocated ());

	// Roll back free pointer to start of arena.
	linearAllocator->reset ();

	EXPECT_EQ (0, linearAllocator->totalAllocated ());
}

//---------------------------------------------------------------------------------------
// memory::align_forward() Tests
//---------------------------------------------------------------------------------------
TEST (Memory, align_forward0)
{
	const size_t align = 0;
	void * const expected = (void *)0x0;
	EXPECT_EQ (expected, memory::align_forward ((void *)0x0, align));
}

TEST (Memory, align_forward2)
{
	const size_t align = 2;

	void * const expected1 = (void *)0x2;
	EXPECT_EQ (expected1, memory::align_forward ((void *)0x1, align));
	EXPECT_EQ (expected1, memory::align_forward ((void *)0x2, align));

	void * const expected2 = (void *)0x4;
	EXPECT_EQ (expected2, memory::align_forward ((void *)0x3, align));
	EXPECT_EQ (expected2, memory::align_forward ((void *)0x4, align));
}

TEST (Memory, align_forward4)
{
	const size_t align = 4;

	void * const expected1 = (void *)0x4;
	EXPECT_EQ (expected1, memory::align_forward((void *)0x1, align));
	EXPECT_EQ (expected1, memory::align_forward((void *)0x2, align));
	EXPECT_EQ (expected1, memory::align_forward((void *)0x3, align));
	EXPECT_EQ (expected1, memory::align_forward((void *)0x4, align));


	void * const expected2 = (void *)0x8;
	EXPECT_EQ (expected2, memory::align_forward((void *)0x5, align));
	EXPECT_EQ (expected2, memory::align_forward((void *)0x6, align));
	EXPECT_EQ (expected2, memory::align_forward((void *)0x7, align));
	EXPECT_EQ (expected2, memory::align_forward((void *)0x8, align));
}

TEST (Memory, align_forward8)
{
	const size_t align = 8;
	void * const expected = (void *)0x8;
	EXPECT_EQ (expected, memory::align_forward((void *)0x1, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x2, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x3, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x4, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x5, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x6, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x7, align));
	EXPECT_EQ (expected, memory::align_forward((void *)0x8, align));
}

TEST (Memory, align_forward_large)
{
	const size_t align = 4;

	void * const expected = (void *)0xFFFFFFFFFFFF0004;
	EXPECT_EQ (expected, memory::align_forward ((void *)0xFFFFFFFFFFFF0001, align));
	EXPECT_EQ (expected, memory::align_forward ((void *)0xFFFFFFFFFFFF0002, align));
	EXPECT_EQ (expected, memory::align_forward ((void *)0xFFFFFFFFFFFF0003, align));
	EXPECT_EQ (expected, memory::align_forward ((void *)0xFFFFFFFFFFFF0004, align));
}
