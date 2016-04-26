###########################################################
## CMAKE SETUP
###########################################################

cmake_minimum_required(VERSION 2.8.7)
project(ipc-bench)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

###########################################################
## COMMON SOURCES
###########################################################

set(IPC_BENCH_COMMON_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/common/utility.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/benchmarks.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/signals.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/arguments.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/process.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/sockets.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/parent.c
)

###########################################################
## DEPENDENCIES
###########################################################

find_package(PkgConfig REQUIRED)

## ZMQ

pkg_check_modules(ZMQ libzmq)

###########################################################
## SUBDIRECTORIES
###########################################################

add_subdirectory(tcp)
add_subdirectory(shm)
add_subdirectory(mmap)
add_subdirectory(fifo)
add_subdirectory(pipes)
add_subdirectory(domain)

if (ZMQ_FOUND)
	add_subdirectory(zmq)
endif()
