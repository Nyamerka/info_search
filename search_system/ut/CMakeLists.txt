add_executable(search_system_ut search_system_ut.cpp)
target_link_libraries(search_system_ut GTest::gtest_main)
target_include_directories(search_system_ut PRIVATE ${CMAKE_SOURCE_DIR})
include(GoogleTest)
gtest_discover_tests(search_system_ut)


