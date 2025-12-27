add_library(search_system INTERFACE)
target_include_directories(search_system INTERFACE ${CMAKE_SOURCE_DIR})

# Shared library for FFI (Python/Go bindings)
add_library(search_engine SHARED c_api.cpp)
target_include_directories(search_engine PUBLIC ${CMAKE_SOURCE_DIR})
set_target_properties(search_engine PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "search_engine"
)

add_subdirectory(ut)
