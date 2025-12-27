add_library(stemmer INTERFACE)
target_include_directories(stemmer INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

