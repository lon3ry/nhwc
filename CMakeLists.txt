cmake_minimum_required(VERSION 3.11)
project("nhwc")
add_executable(main src/main.cpp)
set_target_properties(main PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

enable_testing()
find_package(Python3 COMPONENTS Interpreter)
if (Python3_Interpreter_FOUND)
    add_test(NAME driver COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/2q-test.py" "$<TARGET_FILE:main>"
    )
endif()