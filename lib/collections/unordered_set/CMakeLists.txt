# Header-only library
add_library(collections_unordered_set INTERFACE)
target_include_directories(collections_unordered_set INTERFACE ${CMAKE_SOURCE_DIR})
target_link_libraries(collections_unordered_set INTERFACE collections_unordered_map)

# Tests
add_executable(unordered_set_ut ut/unordered_set_ut.cpp)
target_link_libraries(unordered_set_ut PRIVATE collections_unordered_set types_string GTest::gtest_main)
target_include_directories(unordered_set_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(unordered_set_ut)

