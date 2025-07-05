#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <string_view>
#include "SharedString.h"


template<typename T>
struct is_like_string_8 : is_like_string<T, char> {};

template<typename T>
struct is_like_string_16 : is_like_string<T, std::int16_t> {};


TEST(Static, shared_is_like_string) {

    EXPECT_EQ(is_like_string_8<SharedString<char>>::value, true);
    EXPECT_EQ(is_like_string_8<SharedString<std::int16_t>>::value, false);
    EXPECT_EQ(is_like_string_16<SharedString<std::int16_t>>::value, true);
    EXPECT_EQ(is_like_string_16<SharedString<char>>::value, false);

}


TEST(Static, std_string_is_like_string) {

    EXPECT_EQ(is_like_string_8<std::basic_string<char>>::value, true);
    EXPECT_EQ(is_like_string_8<std::basic_string<std::int16_t>>::value, false);
    EXPECT_EQ(is_like_string_16<std::basic_string<std::int16_t>>::value, true);
    EXPECT_EQ(is_like_string_16<std::basic_string<char>>::value, false);

}


TEST(Static, std_string_view_is_like_string) {

    EXPECT_EQ(is_like_string_8<std::basic_string_view<char>>::value, true);
    EXPECT_EQ(is_like_string_8<std::basic_string_view<std::int16_t>>::value, false);
    EXPECT_EQ(is_like_string_16<std::basic_string_view<std::int16_t>>::value, true);
    EXPECT_EQ(is_like_string_16<std::basic_string_view<char>>::value, false);

}
