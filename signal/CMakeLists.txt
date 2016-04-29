###########################################################
## TARGETS
###########################################################

add_executable(signal-client client.c)
add_executable(signal-server server.c)
add_executable(signal signal.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(signal-client ipc-bench-common)
target_link_libraries(signal-server ipc-bench-common)
target_link_libraries(signal ipc-bench-common)
