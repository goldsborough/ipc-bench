###########################################################
## DEPENDENCIES
###########################################################

if (NOT ZMQ_FOUND)
	message(FATAL_ERROR "ZMQ not found!")
endif()

link_directories(${ZMQ_LIBRARY_DIRS})
include_directories(${ZMQ_INCLUDE_DIRS})

###########################################################
## TARGETS
###########################################################

add_executable(zmq-server server.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(zmq-client client.c ${IPC_BENCH_COMMON_SOURCES})
add_executable(zeromq zmq.c ${IPC_BENCH_COMMON_SOURCES})

target_link_libraries(zmq-server ${ZMQ_LIBRARIES})
target_link_libraries(zmq-client zmq ${ZMQ_LIBRARIES})
target_link_libraries(zeromq zmq ${ZMQ_LIBRARIES})
