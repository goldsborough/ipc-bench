###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 2.8.7)
project(ipc-bench)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

###########################################################
## DEPENDENCIES
###########################################################

find_package(PkgConfig REQUIRED)

## ZMQ

pkg_check_modules(ZMQ libzmq)

###########################################################
## SUBDIRECTORIES
###########################################################

add_subdirectory(common)
add_subdirectory(tcp)
add_subdirectory(shm)
add_subdirectory(mmap)
add_subdirectory(fifo)
add_subdirectory(pipe)
add_subdirectory(domain)
add_subdirectory(mq)
add_subdirectory(signal)

if (ZMQ_FOUND)
	add_subdirectory(zmq)
endif()
