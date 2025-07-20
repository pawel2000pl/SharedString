#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "SharedString.h"



TEST(Stream, BaseWrite) {

    std::ostringstream stream;

    SharedString<char> str1 = "abc";
    SharedString<char> str2 = "def";
    SharedString<char> str3 = "ghj";
    SharedString<char> str4 = "";
    SharedString<char> str5 = "klm";

    stream << str1;
    stream << str2 << str3 << str4;
    stream << str5;

    EXPECT_EQ(stream.str().compare("abcdefghjklm"), 0);

}


TEST(Stream, BaseRead) {

    const char* test = "abc\ndef\nghj\n";

    std::istringstream stream(test);

    SharedString<char> str1;
    SharedString<char> str2;
    SharedString<char> str3;
    SharedString<char> str4;
    
    stream >> str1;
    stream >> str2 >> str3 >> str4;

    EXPECT_EQ(str1.compare("abc"), 0);
    EXPECT_EQ(str2.compare("def"), 0);
    EXPECT_EQ(str3.compare("ghj"), 0);
    EXPECT_EQ(str4.compare(""), 0);


}

TEST(Stream, LongRead) {

    const char* test = "abc\n12345678901234567890123456789012345678901234567890123456789012345678901234567890\nghj\n";

    std::istringstream stream(test);

    SharedString<char> str1;
    SharedString<char> str2;
    SharedString<char> str3;
    SharedString<char> str4;
    
    stream >> str1;
    stream >> str2 >> str3 >> str4;

    EXPECT_EQ(str1.compare("abc"), 0);
    EXPECT_EQ(str2.compare("12345678901234567890123456789012345678901234567890123456789012345678901234567890"), 0);
    EXPECT_EQ(str3.compare("ghj"), 0);
    EXPECT_EQ(str4.compare(""), 0);


}

