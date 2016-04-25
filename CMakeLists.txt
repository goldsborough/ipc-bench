cmake_minimum_required(VERSION 2.8.7)
project(ipc-bench)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(IPC_BENCH_COMMON_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/common/utility.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/benchmarks.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/signals.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/arguments.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/process.c
)

#add_subdirectory(common)
message(STATUS ${IPC_BENCH_COMMON_SOURCES})
#add_subdirectory(tcp)
#add_subdirectory(shm)
add_subdirectory(mmap)
#add_subdirectory(fifos)
#add_subdirectory(pipes)
