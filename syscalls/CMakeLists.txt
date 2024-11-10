# Включаем язык ассемблера
enable_language(ASM)
set(ASM_OPTIONS "-x assembler-with-cpp")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ASM_OPTIONS}")

set(SOURCES "context/program/context.S" "context/context.cpp")

# Создание библиотеки
add_library(syscalls ${SOURCES})

# Установка стандарта C++
target_compile_features(syscalls PRIVATE cxx_std_23)