###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 3.2)
project(vector)

###########################################################
## LIBRARY
###########################################################

add_library(vector SHARED vector.c)
add_library(vector-static STATIC vector.c)

###########################################################
## EXECUTABLES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(vector-test ${CMAKE_CURRENT_SOURCE_DIR}/test/test.c)
add_executable(vector-example ${CMAKE_CURRENT_SOURCE_DIR}/test/example.c)

target_link_libraries(vector-test vector)
target_link_libraries(vector-example vector)

###########################################################
## COMPILER FLAGS
###########################################################

target_compile_options(vector PUBLIC -O3 -Os -g)
