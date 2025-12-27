add_executable(stemmer_ut stemmer_ut.cpp)
target_link_libraries(stemmer_ut GTest::gtest_main)
target_include_directories(stemmer_ut PRIVATE ${CMAKE_SOURCE_DIR})
include(GoogleTest)
gtest_discover_tests(stemmer_ut)

