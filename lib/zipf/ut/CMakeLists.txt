add_executable(zipf_ut zipf_ut.cpp)
target_link_libraries(zipf_ut GTest::gtest_main)
target_include_directories(zipf_ut PRIVATE ${CMAKE_SOURCE_DIR})
include(GoogleTest)
gtest_discover_tests(zipf_ut)

