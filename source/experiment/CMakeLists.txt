###########################################################
## TARGETS
###########################################################

add_executable(experiment-client client.c)
add_executable(experiment-server server.c)
add_executable(experiment experiment.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(experiment-client ipc-bench-common)
target_link_libraries(experiment-server ipc-bench-common)
target_link_libraries(experiment ipc-bench-common)
