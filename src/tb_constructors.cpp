#include <gtest/gtest.h>
#include "SharedString.h"

TEST(Constructors, ConstChar) {

    const char* test_str = "test string";

    SharedString str1 = test_str;

    EXPECT_EQ(str1.compare(test_str), 0);

}
