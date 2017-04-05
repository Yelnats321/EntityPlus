cmake_minimum_required (VERSION 3.0.2)

add_library(EntityPlus INTERFACE)
target_include_directories(EntityPlus INTERFACE ${CMAKE_SOURCE_DIR})

add_subdirectory(entityplus/test)
add_subdirectory(entityplus/example)
