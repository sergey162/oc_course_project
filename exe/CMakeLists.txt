file(GLOB_RECURSE SOURCES "*.cpp")

add_library(exe ${SOURCES})

target_compile_features(exe PRIVATE cxx_std_23)