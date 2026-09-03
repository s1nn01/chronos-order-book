cmake_minimum_required(VERSION 3.20)

project(chronos LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(enable_chronos_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()
endfunction()

add_library(chronos_core
    src/order.cpp
    src/parser.cpp
    src/order_book.cpp
)

target_include_directories(chronos_core PUBLIC include)
enable_chronos_warnings(chronos_core)

add_executable(chronos src/main.cpp)
target_link_libraries(chronos PRIVATE chronos_core)
enable_chronos_warnings(chronos)

enable_testing()

add_executable(chronos_tests tests/test_main.cpp)
target_link_libraries(chronos_tests PRIVATE chronos_core)
enable_chronos_warnings(chronos_tests)
add_test(NAME chronos_tests COMMAND chronos_tests)

add_executable(chronos_benchmark benchmarks/benchmark.cpp)
target_link_libraries(chronos_benchmark PRIVATE chronos_core)
enable_chronos_warnings(chronos_benchmark)
