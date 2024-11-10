file(GLOB_RECURSE SOURCES context/program/context.S "*.cpp")



add_library(syscalls ${SOURCES})