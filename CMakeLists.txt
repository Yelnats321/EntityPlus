cmake_minimum_required (VERSION 3.0.2)

set(PROJECT EntityPlus)
project(${PROJECT})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT} INTERFACE)
target_include_directories(${PROJECT} INTERFACE ${CMAKE_SOURCE_DIR})

file(GLOB SOURCES "entityplus/*.h" "entityplus/*.cpp" "entityplus/*.impl")
add_custom_target(Sources SOURCES ${SOURCES})

add_subdirectory(entityplus/test)
add_subdirectory(entityplus/example)
