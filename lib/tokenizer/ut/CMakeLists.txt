add_executable(tokenizer_ut tokenizer_ut.cpp)
target_link_libraries(tokenizer_ut GTest::gtest_main)
target_include_directories(tokenizer_ut PRIVATE ${CMAKE_SOURCE_DIR})
include(GoogleTest)
gtest_discover_tests(tokenizer_ut)

