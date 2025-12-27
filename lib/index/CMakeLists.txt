add_library(index INTERFACE)
target_include_directories(index INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

