cmake_minimum_required(VERSION 3.20)

project(chronos LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(chronos
    src/main.cpp
    src/order.cpp
    src/parser.cpp
)

target_include_directories(chronos PRIVATE
    include
)

if(MSVC)
    target_compile_options(chronos PRIVATE /W4 /permissive-)
else()
    target_compile_options(chronos PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

