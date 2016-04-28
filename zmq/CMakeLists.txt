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

add_executable(zmq-client client.c)
add_executable(zmq-server server.c)
add_executable(zeromq zmq.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(zmq-server ipc-bench-common ${ZMQ_LIBRARIES})
target_link_libraries(zmq-client ipc-bench-common ${ZMQ_LIBRARIES})
target_link_libraries(zeromq ipc-bench-common ${ZMQ_LIBRARIES})
