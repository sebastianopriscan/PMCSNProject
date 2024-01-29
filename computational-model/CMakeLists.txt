cmake_minimum_required(VERSION 3.24)
project(computational-model C)

set(CMAKE_C_STANDARD 11)

add_library(computational-model 
  src/deserializer/deserializer.c
  src/park_simulation/sim_state.c
  src/park_simulation/behaviors/client_behaviors.c
  src/park_simulation/behaviors/ride_behaviors.c
  src/park_simulation/behaviors/show_behaviors.c
  src/park_simulation/run_simulation.c
  src/models/utils.c
)

add_library(lambda_evaluator
  src/lambda-evaluator/lambda_evaluator.c
)

target_link_libraries(lambda_evaluator PRIVATE NextEventSimulationFramework)

# Required as the json.h header file is automatically generated during build
target_include_directories(computational-model PRIVATE ../build/libs/json-c)

target_link_libraries(computational-model PRIVATE NextEventSimulationFramework)
target_link_libraries(computational-model PRIVATE json-c)

add_executable(lambda-evaluator-executable 
  src/lambda-evaluator/main.c
  src/deserializer/deserializer.c
)

target_link_libraries(lambda-evaluator-executable PRIVATE lambda_evaluator)
target_link_libraries(lambda-evaluator-executable PRIVATE NextEventSimulationFramework)
target_link_libraries(lambda-evaluator-executable PRIVATE json-c)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  enable_testing()
  add_subdirectory(tests)
endif()