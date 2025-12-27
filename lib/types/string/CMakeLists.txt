# Header-only library
add_library(types_string INTERFACE)
target_include_directories(types_string INTERFACE ${CMAKE_SOURCE_DIR})

# Tests
add_executable(string_ut ut/string_ut.cpp)
target_link_libraries(string_ut PRIVATE types_string GTest::gtest_main)
target_include_directories(string_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(string_ut)

