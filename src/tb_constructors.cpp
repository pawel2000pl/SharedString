#include <gtest/gtest.h>
#include <cstring>
#include "SharedString.h"


TEST(Constructors, Allocate) {

    SharedString str1(125);

    str1.push_back('a');
    str1.push_back('b');
    str1.push_back('c');

    EXPECT_EQ(str1.compare("abc"), 0);
    EXPECT_EQ(str1.capacity(), 125);
    EXPECT_EQ(strcmp(str1.c_str(), "abc"), 0);
    EXPECT_EQ(str1.capacity(), 125);

}


TEST(Constructors, ConstChar) {

    const char* test_str = "test string";

    SharedString str1 = test_str;

    EXPECT_EQ(str1.compare(test_str), 0);
    EXPECT_EQ(strcmp(str1.c_str(), test_str), 0);
    EXPECT_EQ(str1.references_count(), 1);

}
