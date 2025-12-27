# Header-only library
add_library(collections_list INTERFACE)
target_include_directories(collections_list INTERFACE ${CMAKE_SOURCE_DIR})

# Tests
add_executable(list_ut ut/list_ut.cpp)
target_link_libraries(list_ut PRIVATE collections_list GTest::gtest_main)
target_include_directories(list_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(list_ut)

