add_executable(heap_ut heap_ut.cpp)
target_link_libraries(heap_ut GTest::gtest_main heap)
target_include_directories(heap_ut PRIVATE ${CMAKE_SOURCE_DIR})
add_test(NAME HeapTests COMMAND heap_ut)

