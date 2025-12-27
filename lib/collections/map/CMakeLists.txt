# Header-only library
add_library(collections_map INTERFACE)
target_include_directories(collections_map INTERFACE ${CMAKE_SOURCE_DIR})

# Tests
add_executable(map_ut ut/map_ut.cpp)
target_link_libraries(map_ut PRIVATE collections_map types_string GTest::gtest_main)
target_include_directories(map_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(map_ut)

