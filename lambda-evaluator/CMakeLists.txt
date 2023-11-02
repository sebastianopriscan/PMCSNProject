cmake_minimum_required(VERSION 3.24)
project(lambda-evaluator C)

set(CMAKE_C_STANDARD 11)

add_executable(lambda-evaluator src/main.c)

target_link_libraries(lambda-evaluator PRIVATE NextEventSimulationFramework)
target_link_libraries(lambda-evaluator PRIVATE RandomVariates m)