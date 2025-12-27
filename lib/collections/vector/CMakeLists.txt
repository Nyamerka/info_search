# Header-only library
add_library(collections_vector INTERFACE)
target_include_directories(collections_vector INTERFACE ${CMAKE_SOURCE_DIR})

# Tests
add_executable(vector_ut ut/vector_ut.cpp)
target_link_libraries(vector_ut PRIVATE collections_vector GTest::gtest_main)
target_include_directories(vector_ut PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(vector_ut)

