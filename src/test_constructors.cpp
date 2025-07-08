#include <gtest/gtest.h>
#include <cstring>
#include <list>
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


TEST(Constructors, InitializerList) {

    SharedString str1(std::initializer_list({'a', 'b', 'c'}));

    EXPECT_EQ(str1.compare("abc"), 0);
    EXPECT_EQ(strcmp(str1.c_str(), "abc"), 0);

}


TEST(Constructors, IteratorWithPointers) {

    const char* test_str = "test string";

    SharedString str1(test_str, test_str+strlen(test_str));

    EXPECT_EQ(str1.compare(test_str), 0);
    EXPECT_EQ(strcmp(str1.c_str(), test_str), 0);
    EXPECT_EQ(str1.references_count(), 1);

}


TEST(Constructors, IteratorWithList) {

    const char* test_str = "test string";
    std::list<char> source;
    const char* i = test_str;
    while (*i) source.push_back(*(i++));

    SharedString str1(std::begin(source), std::end(source));

    EXPECT_EQ(str1.compare(test_str), 0);
    EXPECT_EQ(strcmp(str1.c_str(), test_str), 0);
    EXPECT_EQ(str1.references_count(), 1);

}


TEST(Constructors, CopyConstructorFromConstA) {

    const char* test_str = "test string";

    // const-buf source with detected zero at the end
    SharedString<char>* source = new SharedString<char>(test_str);
    EXPECT_EQ(source->references_count(), 1);

    SharedString<char>* copy1 = new SharedString<char>(*source);
    EXPECT_EQ(source->references_count(), 2);
    EXPECT_EQ(copy1->references_count(), 2);

    delete source;
    EXPECT_EQ(copy1->references_count(), 1);
    delete copy1;

}


TEST(Constructors, CopyConstructorFromConstB) {

    const char* test_str = "test string";

    // const-buf source with detected zero at the end
    SharedString<char>* source = new SharedString<char>(test_str);
    EXPECT_EQ(source->references_count(), 1);

    SharedString<char>* copy1 = new SharedString<char>(*source);
    EXPECT_EQ(source->references_count(), 2);
    EXPECT_EQ(copy1->references_count(), 2);

    delete copy1;
    EXPECT_EQ(source->references_count(), 1);
    delete source;

}


TEST(Constructors, MoveConstructorFromConst) {

    const char* test_str = "test string";

    // const-buf source with detected zero at the end
    SharedString<char>* source = new SharedString<char>(test_str);
    EXPECT_EQ(source->references_count(), 1);
    EXPECT_EQ(source->is_moved(), false);

    SharedString<char>* copy1 = new SharedString<char>(std::move(*source));
    EXPECT_EQ(source->is_moved(), true);
    EXPECT_EQ(copy1->references_count(), 1);

    delete source;
    EXPECT_EQ(copy1->references_count(), 1);
    delete copy1;

}
