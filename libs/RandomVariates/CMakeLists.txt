cmake_minimum_required(VERSION 3.24)
project(RandomVariates C)

set(CMAKE_C_STANDARD 11)

add_library(RandomVariates STATIC rngs.c rngs.h rvgs.c rvgs.h)

target_include_directories (RandomVariates PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})