add_executable(lzw_ut lzw_ut.cpp)
target_link_libraries(lzw_ut GTest::gtest_main)
target_include_directories(lzw_ut PRIVATE ${CMAKE_SOURCE_DIR})
include(GoogleTest)
gtest_discover_tests(lzw_ut)


