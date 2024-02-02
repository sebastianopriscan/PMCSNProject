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
target_link_libraries(production_runs PRIVATE NextEventSimulationFramework)
target_link_libraries(production_runs PRIVATE m)

set_target_properties(production_runs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

add_custom_command(TARGET production_runs POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:production_runs>
            ${CMAKE_SOURCE_DIR}/bin/
    COMMENT "Copying MyExecutable to bin directory"
)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  enable_testing()
  add_subdirectory(tests)
endif()