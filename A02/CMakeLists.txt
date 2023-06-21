cmake_minimum_required(VERSION 3.25)
project(A02 C)

set(CMAKE_C_STANDARD 17)

add_executable(A02 logging.h logging.c kosmos-sem.c kosmos-mcv.c)
