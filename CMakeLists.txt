cmake_minimum_required(VERSION 3.16)
project(ex4 C)

set(CMAKE_C_FLAGS -pthread)
set(CMAKE_C_STANDARD 99)

add_executable(ex4 threadPool.c threadPool.h main.c osqueue.c osqueue.h)