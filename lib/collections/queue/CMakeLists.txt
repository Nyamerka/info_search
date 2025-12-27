add_library(queue INTERFACE)
target_include_directories(queue INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

