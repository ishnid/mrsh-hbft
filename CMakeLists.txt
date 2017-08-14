cmake_minimum_required(VERSION 3.5)

#set(CMAKE_VERBOSE_MAKEFILE on)

project(mrsh_hbft C)

set(CMAKE_C_FLAGS "-w -std=c99 -O3 -D_BSD_SOURCE")

set(SOURCE_FILES
    src/bloomfilter.c
    src/fnv.c
    src/hashing.c
    src/helper.c
    src/main.c src/test.c src/bloomfiltertree.c header/filehash.h src/filehash.c src/fingerprint.c
        src/fingerprintList.c)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(mrsh_hbft ${SOURCE_FILES})
target_link_libraries(mrsh_hbft m)
