cmake_minimum_required(VERSION 3.24)
project(computational-model C)

set(CMAKE_C_STANDARD 11)

add_executable(computational-model 
  src/deserializer/deserializer.c src/deserializer/deserializer.h
  src/main.c
)

# Required as the json.h header file is automatically generated during build
target_include_directories(computational-model PRIVATE ../build/libs/json-c)

target_link_libraries(computational-model PRIVATE NextEventSimulationFramework)
target_link_libraries(computational-model PRIVATE json-c)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  enable_testing()
  add_subdirectory(tests)
endif()