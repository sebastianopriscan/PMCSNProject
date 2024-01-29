cmake_minimum_required(VERSION 3.24)
project(production_runs C)

set(CMAKE_C_STANDARD 11)

add_executable(production_runs
    src/main.c
)

# Required as the json.h header file is automatically generated during build
# target_include_directories(production_runs PRIVATE ../)

target_link_libraries(production_runs PRIVATE computational-model)
target_link_libraries(production_runs PRIVATE lambda_evaluator)
target_link_libraries(production_runs PRIVATE pthread)
target_link_libraries(production_runs PRIVATE NextEventSimulationFramework)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  enable_testing()
  add_subdirectory(tests)
endif()