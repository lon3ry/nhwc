cmake_minimum_required(VERSION 3.11)
project("nhwc")
add_executable(main src/main.cpp)
set_target_properties(main PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)