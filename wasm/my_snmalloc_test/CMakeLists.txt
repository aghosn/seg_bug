cmake_minimum_required(VERSION 3.8.0)
project(snmalloc-test LANGUAGES C CXX)

set(MY_TARGET my_main)
set(MY_SOURCES src/main.cc)
include_directories(AFTER "${CMAKE_SOURCE_DIR}/external/snmalloc/src")

add_compile_options(-mcx16)
add_executable(${MY_TARGET} ${MY_SOURCES})
