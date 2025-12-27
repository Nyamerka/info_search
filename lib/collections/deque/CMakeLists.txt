add_library(deque INTERFACE)
target_include_directories(deque INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

