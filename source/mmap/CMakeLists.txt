###########################################################
## TARGETS
###########################################################

add_executable(mmap-client client.c)
add_executable(mmap-server server.c)
add_executable(mmap mmap.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(mmap-client ipc-bench-common)
target_link_libraries(mmap-server ipc-bench-common)
target_link_libraries(mmap ipc-bench-common)
