cmake_minimum_required(VERSION 3.8.0)
project(snmalloc-test LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)



## ———————————————————————————— Single Allocator ———————————————————————————— ##
set(MY_TARGET my_main)
set(MY_SOURCES src/main.cc)

set(SNMALLOC_ONLY_HEADER_LIBRARY ON)
add_subdirectory(external/snmalloc EXCLUDE_FROM_ALL)

#include_directories(AFTER "${CMAKE_SOURCE_DIR}/external/snmalloc/src")

add_compile_options(-mcx16)
add_executable(${MY_TARGET} ${MY_SOURCES})
target_link_libraries(${MY_TARGET} snmalloc)

## —————————————————————————— Multiple Allocators ——————————————————————————— ## 

set(SANDBOX sandbox)
set(SANDBOX_SOURCES src/sandbox.cc)
add_executable(${SANDBOX} ${SANDBOX_SOURCES})
target_link_libraries(${SANDBOX} snmalloc)

