# Header-only library
add_library(collections_set INTERFACE)
target_include_directories(collections_set INTERFACE ${CMAKE_SOURCE_DIR})
target_link_libraries(collections_set INTERFACE collections_map)

# Tests
add_executable(set_ut ut/set_ut.cpp)
target_link_libraries(set_ut PRIVATE collections_set types_string GTest::gtest_main)
target_include_directories(set_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(set_ut)

