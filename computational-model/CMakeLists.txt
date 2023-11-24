cmake_minimum_required(VERSION 3.24)
project(computational-model C)

set(CMAKE_C_STANDARD 11)

add_executable(computational-model src/main.c)

target_link_libraries(computational-model PRIVATE NextEventSimulationFramework)
