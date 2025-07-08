#include <gtest/gtest.h>
#include "SharedString.h"

using TestString = SharedString<char>;

TEST(Split, Basic) {

    TestString str1("0123$#456$#789");

    std::list<TestString> splitted = str1.split("$#");
    auto it = std::begin(splitted);

    EXPECT_EQ(str1.compare("0123$#456$#789"), 0);
    EXPECT_EQ(str1.references_count(), 4);
    EXPECT_EQ(splitted.size(), 3);
    EXPECT_EQ((it++)->compare("0123"), 0);
    EXPECT_EQ((it++)->compare("456"), 0);
    EXPECT_EQ((it++)->compare("789"), 0);

}


TEST(Split, Empties) {

    TestString str1("$#0123$#$#456$#789$#");

    std::list<TestString> splitted = str1.split("$#");
    auto it = std::begin(splitted);

    EXPECT_EQ(str1.references_count(), 7);
    EXPECT_EQ(splitted.size(), 6);
    EXPECT_EQ((it++)->compare(""), 0);
    EXPECT_EQ((it++)->compare("0123"), 0);
    EXPECT_EQ((it++)->compare(""), 0);
    EXPECT_EQ((it++)->compare("456"), 0);
    EXPECT_EQ((it++)->compare("789"), 0);
    EXPECT_EQ((it++)->compare(""), 0);

}


TEST(Split, Limit) {

    TestString str1("$#0123$#$#456$#789$#");

    std::list<TestString> splitted = str1.split("$#", TestString::npos, 3);
    auto it = std::begin(splitted);

    EXPECT_EQ(str1.references_count(), 5);
    EXPECT_EQ(splitted.size(), 4);
    EXPECT_EQ((it++)->compare(""), 0);
    EXPECT_EQ((it++)->compare("0123"), 0);
    EXPECT_EQ((it++)->compare(""), 0);
    EXPECT_EQ((it++)->compare("456$#789$#"), 0);

}

