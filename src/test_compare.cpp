#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include <string_view>
#include <algorithm>
#include "SharedString.h"


TEST(Compare, Basic) {

    SharedString str1("abcdef");
    SharedString str2("abcdef");
    std::string str3("abcdef");
    std::string_view str4("abcdef");
    SharedString str5(str2);

    EXPECT_EQ(str1.compare(str1), 0);
    EXPECT_EQ(str1.compare(str2), 0);
    EXPECT_EQ(str2.compare(str1), 0);

    EXPECT_EQ(str2.compare(str5), 0);
    EXPECT_EQ(str5.compare(str2), 0);
    EXPECT_EQ(str1.compare(str5), 0);
    EXPECT_EQ(str5.compare(str1), 0);

    EXPECT_EQ(str1.compare(str3), 0);
    EXPECT_EQ(str1.compare(str4), 0);
    EXPECT_EQ(str1.compare("abcdef"), 0);

}


TEST(Compare, NonZeroTerminated) {

    SharedString str1("abcdee", 5);
    SharedString str2("abcdef", 5);

    EXPECT_EQ(str1.compare(str1), 0);
    EXPECT_EQ(str1.compare(str2), 0);
    EXPECT_EQ(str2.compare(str1), 0);

}


TEST(Compare, various_lengths) {

    SharedString str1("abcdef");
    SharedString str2("abcde");

    EXPECT_EQ(str1.compare(str2), -1);
    EXPECT_EQ(str2.compare(str1), 1);

}



TEST(Compare, various_lengths_non_zero_terminated) {

    SharedString str1("abcdefgh", 6);
    SharedString str2("abcdefgh", 5);

    EXPECT_EQ(str1.compare(str2), -1);
    EXPECT_EQ(str2.compare(str1), 1);

}


TEST(Compare, empty_str) {
    
    SharedString str1("");
    SharedString str2("daaa", (std::size_t)0);

    EXPECT_EQ(str1.compare(str2), 0);
    EXPECT_EQ(str2.compare(str1), 0);

    EXPECT_EQ(str1.compare(""), 0);
    EXPECT_EQ(str1.compare("a"), 1);
    EXPECT_EQ(str1.compare("aaaaaAAAAAAAAaaAAaaAAaaaaaaaaaaAAAAAAAAAAAAAAaaaaaaaaaAAAAaaaaAAAaAAAaAAAaaAAa"), 1);

    EXPECT_EQ(str2.compare(""), 0);
    EXPECT_EQ(str2.compare("a"), 1);
    EXPECT_EQ(str2.compare("aaaaaAAAAAAAAaaAAaaAAaaaaaaaaaaAAAAAAAAAAAAAAaaaaaaaaaAAAAaaaaAAAaAAAaAAAaaAAa"), 1);

}


TEST(Compare, OperatorsEq) {

    SharedString str1("abcdef");
    SharedString str2("abcdef");
    std::string str3("abcdef");
    std::string_view str4("abcdef");

    EXPECT_TRUE(str1 == str2);
    EXPECT_TRUE(str1 == str3);
    EXPECT_TRUE(str1 == str4);
    EXPECT_TRUE(str1 == "abcdef");

}


TEST(Compare, OperatorsNeq) {

    SharedString str1("abcdef");
    SharedString str2("bcdef");
    std::string str3("bcdef");
    std::string_view str4("bcdef");

    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str1 < str3);
    EXPECT_TRUE(str1 < str4);
    EXPECT_TRUE(str1 < "bcdef");

    EXPECT_TRUE(str2 > str1);
    EXPECT_TRUE(str3 > str1);
    EXPECT_TRUE(str4 > str1);
    EXPECT_TRUE("bcdef" > str1);

}