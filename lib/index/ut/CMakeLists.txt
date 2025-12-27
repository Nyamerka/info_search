add_executable(boolean_index_ut boolean_index_ut.cpp)
target_link_libraries(boolean_index_ut GTest::gtest_main)
target_include_directories(boolean_index_ut PRIVATE ${CMAKE_SOURCE_DIR})
include(GoogleTest)
gtest_discover_tests(boolean_index_ut)

