#include <gtest/gtest.h>
#include "SharedString.h"

using TestString = SharedString<char>;

TEST(Substr, Basic) {

    TestString str1("0123456789");

    EXPECT_EQ(str1.substr(0).compare(str1), 0);
    EXPECT_EQ(str1.substr(2).compare("23456789"), 0);
    EXPECT_EQ(str1.substr(2, 3).compare("234"), 0);

}


TEST(Substr, Ranges) {

    TestString str1("0123456789");

    EXPECT_EQ(str1.substr(999).size(), 0);
    EXPECT_EQ(str1.substr(999, 1).size(), 0);
    EXPECT_EQ(str1.substr(2, 3000).compare("23456789"), 0);

}


TEST(Substr, ReferencesBase) {

    TestString str1("0123456789");
    TestString str2 = str1.substr(2, 3);

    EXPECT_EQ(str2.compare("234"), 0);
    EXPECT_EQ(str2.references_count(), 2);
    EXPECT_EQ(str1.references_count(), 2);

}


TEST(Substr, ReferencesPtrConst) {

    TestString* str1 = new TestString("0123456789");
    TestString* str2 = new TestString(str1->substr(2, 3));

    EXPECT_EQ(str2->compare("234"), 0);
    EXPECT_EQ(str2->references_count(), 2);
    EXPECT_EQ(str1->references_count(), 2);

    delete str1;

    EXPECT_EQ(str2->compare("234"), 0);
    EXPECT_EQ(str2->references_count(), 1);

    delete str2;

}


TEST(Substr, ReferencesPtrReserved) {

    TestString* str1 = new TestString((char*)"0123456789");
    str1->reserve(15);

    EXPECT_EQ(str1->references_count(), 1);

    TestString* str2 = new TestString(str1->substr(2, 3));

    EXPECT_EQ(str2->compare("234"), 0);
    EXPECT_EQ(str2->references_count(), 2);
    EXPECT_EQ(str1->references_count(), 2);

    str1->push_back("1234567890"); // requires reservation of new buffer
    
    EXPECT_EQ(str1->references_count(), 1);
    EXPECT_EQ(str1->compare("0123456789" "1234567890"), 0);
    EXPECT_EQ(str2->compare("234"), 0);
    EXPECT_EQ(str2->references_count(), 1);

    delete str1;

    EXPECT_EQ(str2->compare("234"), 0);
    EXPECT_EQ(str2->references_count(), 1);

    delete str2;

}


TEST(Substr, ReferencesPtrSubPushed) {

    TestString* str1 = new TestString((char*)"0123456789"); // forces copy
    TestString* str2 = new TestString(str1->substr(2, 3));

    str2->push_back("1234567890"); // requires reservation of new buffer
    
    EXPECT_EQ(str1->references_count(), 1);
    EXPECT_EQ(str1->compare("0123456789"), 0);
    EXPECT_EQ(str2->compare("234" "1234567890"), 0);
    EXPECT_EQ(str2->references_count(), 1);

    delete str2;

    EXPECT_EQ(str1->compare("0123456789"), 0);
    EXPECT_EQ(str1->references_count(), 1);

    delete str1;

}
