cmake_minimum_required(VERSION 3.21)
project(Assembler C)

set(CMAKE_C_STANDARD 99)

add_executable(Assembler main.c cmake-build-debug/assemble.h)
