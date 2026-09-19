cmake_minimum_required(VERSION 3.14)
project("nhwc")
add_executable(main
    src/main.cpp
    src/config.cpp
    src/multi_level_cache.cpp
)
set_target_properties(main PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_library(nhwc INTERFACE)
target_include_directories(nhwc INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(main PRIVATE nhwc)

target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
enable_testing()
add_subdirectory(tests)
