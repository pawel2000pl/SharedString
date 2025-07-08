#include <gtest/gtest.h>
#include <string>
#include "SharedString.h"

// easy switch to std::string
using TestString = SharedString<char>;


TEST(Find, Basic) {

    TestString str1("abc##abc##abc");

    EXPECT_EQ(str1.find("abc", 0), 0);
    EXPECT_EQ(str1.find("abc", 1), 5);
    EXPECT_EQ(str1.find("abc", 5), 5);
    EXPECT_EQ(str1.find("abc", 6), 10);
    EXPECT_EQ(str1.find("abc", 11), TestString::npos);

}


TEST(Find, Overload) {

    TestString str1("abc##abc##abc");

    EXPECT_EQ(str1.find("abc", 200), TestString::npos);
    EXPECT_EQ(str1.find("nope", 0), TestString::npos);

}


TEST(Find, EmptyNeedle) {
    
    TestString str1("abc##abc##abc");

    EXPECT_EQ(str1.find("", 0), 0);
    EXPECT_EQ(str1.find("", 1), 1);
    EXPECT_EQ(str1.find("", 200), TestString::npos);

}


TEST(Find, EmptyHaystack) {
    
    TestString str1("");

    EXPECT_EQ(str1.find("abc", 0), TestString::npos);
    EXPECT_EQ(str1.find("", 1), TestString::npos);
    EXPECT_EQ(str1.find("", 0), 0);
    EXPECT_EQ(str1.find("", 200), TestString::npos);
    EXPECT_EQ(str1.find("abc", 200), TestString::npos);

}


TEST(RFind, Basic) {

    TestString str1("abc##abc##abc");

    EXPECT_EQ(str1.rfind("abc"), 10);
    EXPECT_EQ(str1.rfind("abc", 12), 10);
    EXPECT_EQ(str1.rfind("abc", 11), 10);
    EXPECT_EQ(str1.rfind("abc", 5), 5);
    EXPECT_EQ(str1.rfind("abc", 3), 0);
    EXPECT_EQ(str1.rfind("abc", 2), 0);

}


TEST(RFind, Overload) {

    TestString str1("abc##abc##abc");

    EXPECT_EQ(str1.rfind("abc", 200), 10);
    EXPECT_EQ(str1.rfind("nope", 0), TestString::npos);

}


TEST(RFind, EmptyNeedle) {
    
    TestString str1("abc##abc##abc");

    EXPECT_EQ(str1.rfind(""), str1.size());
    EXPECT_EQ(str1.rfind("", 0), 0);
    EXPECT_EQ(str1.rfind("", 1), 1);
    EXPECT_EQ(str1.rfind("", 200), str1.size());

}


TEST(RFind, EmptyHaystack) {
    
    TestString str1("");

    EXPECT_EQ(str1.rfind("abc", 0), TestString::npos);
    EXPECT_EQ(str1.rfind("", 1), 0);
    EXPECT_EQ(str1.rfind("", 0), 0);
    EXPECT_EQ(str1.rfind("", 200), 0);
    EXPECT_EQ(str1.rfind("abc", 200), TestString::npos);

}
