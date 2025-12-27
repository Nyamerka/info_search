add_library(heap INTERFACE)
target_include_directories(heap INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

