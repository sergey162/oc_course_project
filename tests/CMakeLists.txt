add_executable(tests_asan unit.cpp)
target_link_libraries(tests_asan PRIVATE gtest)

target_compile_options(tests_asan PRIVATE -fsanitize=address)
target_link_options(tests_asan PRIVATE -fsanitize=address)
target_link_libraries(tests_asan PRIVATE exe)
target_link_libraries(tests_asan PRIVATE syscalls)


add_executable(tests_tsan unit.cpp)
target_link_libraries(tests_tsan PRIVATE gtest)

target_compile_options(tests_tsan PRIVATE -fsanitize=thread)
target_link_options(tests_tsan PRIVATE -fsanitize=thread)
target_link_libraries(tests_tsan PRIVATE exe)
target_link_libraries(tests_tsan PRIVATE syscalls)


