###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 2.8.7)
project(ipc-bench)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

# Apple's C standard library already contains the math functions
if(NOT APPLE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lm")
endif()

###########################################################
## DEPENDENCIES
###########################################################

find_package(PkgConfig REQUIRED)

## ZMQ

pkg_check_modules(ZMQ libzeromq)

# Hack for ubuntu: ZMQ lib has a different name, also try to detect this
if (NOT ZMQ_FOUND)
	pkg_check_modules(ZMQ libzmq)
endif()

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

if (NOT APPLE)
	add_subdirectory(eventfd)
endif()

if (ZMQ_FOUND)
	add_subdirectory(zeromq)
endif()
