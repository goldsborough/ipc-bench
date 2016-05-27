###########################################################
## TARGETS
###########################################################

add_executable(shm-sync-client client.c shm-sync-common.c)
add_executable(shm-sync-server server.c shm-sync-common.c)
add_executable(shm-sync shm-sync.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(shm-sync-client ipc-bench-common pthread)
target_link_libraries(shm-sync-server ipc-bench-common pthread)
target_link_libraries(shm-sync ipc-bench-common)
