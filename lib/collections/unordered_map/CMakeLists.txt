# Header-only library
add_library(collections_unordered_map INTERFACE)
target_include_directories(collections_unordered_map INTERFACE ${CMAKE_SOURCE_DIR})

# Tests
add_executable(unordered_map_ut ut/unordered_map_ut.cpp)
target_link_libraries(unordered_map_ut PRIVATE collections_unordered_map types_string GTest::gtest_main)
target_include_directories(unordered_map_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(unordered_map_ut)

