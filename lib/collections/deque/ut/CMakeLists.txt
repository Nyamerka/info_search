add_executable(deque_ut deque_ut.cpp)
target_link_libraries(deque_ut GTest::gtest_main deque)
target_include_directories(deque_ut PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME DequeTests COMMAND deque_ut)

