add_library(zipf INTERFACE)
target_include_directories(zipf INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

