
#include "gtest/gtest.h"

#define TEST_NAME Memory

TEST (TEST_NAME, one)
{
	EXPECT_EQ (1, 1);
}

TEST (TEST_NAME, two)
{
	EXPECT_TRUE (true);
}

TEST (TEST_NAME, three)
{
	EXPECT_TRUE (false);
}
