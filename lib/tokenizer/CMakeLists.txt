add_library(tokenizer INTERFACE)
target_include_directories(tokenizer INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(ut)

