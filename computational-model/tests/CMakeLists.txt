cmake_minimum_required(VERSION 3.24)
project(computational-model C)

include(CTest)

include_directories(../src)

set(CMAKE_C_STANDARD 11)

add_executable(TestDeserializer 
  ../src/deserializer/deserializer.c
  test_deserializer.c
)
target_include_directories(TestDeserializer PRIVATE ../../build/libs/json-c)
target_link_libraries(TestDeserializer PRIVATE json-c)
add_test(NAME TestDeserializer 
        COMMAND TestDeserializer)