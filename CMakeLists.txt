cmake_minimum_required(VERSION 3.14)

project(concurrency_framework)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

set(REQUIRED_CXX_COMPILER "Clang")

set(CXX_COMPILER_MIN_VERSION 17.0)

add_compile_options(-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer)

add_compile_options(-Wno-language-extension-token)

add_compile_options(-Wno-error=unused-command-line-argument)

add_compile_options(-gdwarf-4)


add_compile_options(-Werror)

add_subdirectory(exe)

add_subdirectory(syscalls)

add_executable(my_executable main.cpp)

target_link_libraries(my_executable PRIVATE exe)

target_link_libraries(my_executable PRIVATE syscalls)
