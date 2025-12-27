add_executable(queue_ut queue_ut.cpp)
target_link_libraries(queue_ut GTest::gtest_main queue)
target_include_directories(queue_ut PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME QueueTests COMMAND queue_ut)

