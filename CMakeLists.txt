cmake_minimum_required(VERSION 3.14)

project(concurrency_framework)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

set(REQUIRED_CXX_COMPILER "Clang")

set(CXX_COMPILER_MIN_VERSION 17.0)

add_compile_options(-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer)

add_compile_options(-gdwarf-4)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG f8d7d77
)
FetchContent_MakeAvailable(googletest)


add_compile_options(-Werror)

add_subdirectory(exe)
add_subdirectory(syscalls)
add_subdirectory(tests)

