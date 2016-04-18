cmake_minimum_required(VERSION 2.8.7)
project(ipc-bench)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(tcp)
add_subdirectory(shm)
add_subdirectory(mmap)
add_subdirectory(fifos)
add_subdirectory(pipes)
