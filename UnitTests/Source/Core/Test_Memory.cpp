//
// Test_Memory.cpp
//

#include <gtest/gtest.h>

#include "Engine/Source/Core/Memory.hpp"


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
