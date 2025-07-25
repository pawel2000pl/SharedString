cmake_minimum_required(VERSION 3.14)
project(shared_string_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra -O2)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()


add_executable(
  test_constructors
  src/test_constructors.cpp
)
target_link_libraries(
  test_constructors
  GTest::gtest_main
)


add_executable(
  test_static
  src/test_static.cpp
)
target_link_libraries(
  test_static
  GTest::gtest_main
)


add_executable(
  test_compare
  src/test_compare.cpp
)
target_link_libraries(
  test_compare
  GTest::gtest_main
)


add_executable(
  test_find
  src/test_find.cpp
)
target_link_libraries(
  test_find
  GTest::gtest_main
)


add_executable(
  test_substr
  src/test_substr.cpp
)
target_link_libraries(
  test_substr
  GTest::gtest_main
)


add_executable(
  test_split
  src/test_split.cpp
)
target_link_libraries(
  test_split
  GTest::gtest_main
)


add_executable(
  test_stream
  src/test_stream.cpp
)
target_link_libraries(
  test_stream
  GTest::gtest_main
)


include(GoogleTest)
gtest_discover_tests(test_constructors)
gtest_discover_tests(test_static)
gtest_discover_tests(test_compare)
gtest_discover_tests(test_find)
gtest_discover_tests(test_substr)
gtest_discover_tests(test_split)
gtest_discover_tests(test_stream)
