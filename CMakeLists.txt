cmake_minimum_required(VERSION 3.28.0)
project(conway VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_FLAGS, "-O3")

file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(conway ${SOURCES})