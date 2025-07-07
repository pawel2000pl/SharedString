#include <gtest/gtest.h>
#include "SharedString.h"


TEST(Find, Basic) {

    SharedString str1("abc##abc##abc");

    EXPECT_EQ(str1.find("abc", 0), 0);
    EXPECT_EQ(str1.find("abc", 1), 5);
    EXPECT_EQ(str1.find("abc", 5), 5);
    EXPECT_EQ(str1.find("abc", 6), 10);
    EXPECT_EQ(str1.find("abc", 11), SharedString<char>::npos);

}


TEST(Find, Overload) {

    SharedString str1("abc##abc##abc");

    EXPECT_EQ(str1.find("abc", 200), SharedString<char>::npos);
    EXPECT_EQ(str1.find("nope", 0), SharedString<char>::npos);

}


TEST(Find, EmptyNeedle) {
    
    SharedString str1("abc##abc##abc");

    EXPECT_EQ(str1.find("", 0), 0);
    EXPECT_EQ(str1.find("", 1), 1);
    EXPECT_EQ(str1.find("", 200), SharedString<char>::npos);

}


TEST(Find, EmptyHaystack) {
    
    SharedString str1("");

    EXPECT_EQ(str1.find("abc", 0), SharedString<char>::npos);
    EXPECT_EQ(str1.find("", 1), SharedString<char>::npos);
    EXPECT_EQ(str1.find("", 0), 0);
    EXPECT_EQ(str1.find("", 200), SharedString<char>::npos);
    EXPECT_EQ(str1.find("abc", 200), SharedString<char>::npos);

}


TEST(RFind, Basic) {

    SharedString str1("abc##abc##abc");

    EXPECT_EQ(str1.rfind("abc"), 10);
    EXPECT_EQ(str1.rfind("abc", 12), 10);
    EXPECT_EQ(str1.rfind("abc", 11), 5);
    EXPECT_EQ(str1.rfind("abc", 5), 0);
    EXPECT_EQ(str1.rfind("abc", 3), 0);
    EXPECT_EQ(str1.rfind("abc", 2), SharedString<char>::npos);

}


TEST(RFind, Overload) {

    SharedString str1("abc##abc##abc");

    EXPECT_EQ(str1.rfind("abc", 200), 10);
    EXPECT_EQ(str1.rfind("nope", 0), SharedString<char>::npos);

}
