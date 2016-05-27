###########################################################
## TARGETS
###########################################################

add_executable(experiment-client client.c)
add_executable(experiment-server server.c)
add_executable(experiment experiment.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(experiment-server ipc-bench-common tssx-server)
target_link_libraries(experiment-client ipc-bench-common tssx-client)
target_link_libraries(experiment ipc-bench-common)

###########################################################
## LINKER FLAGS
###########################################################

set_target_properties(
	experiment-server PROPERTIES
	LINK_FLAGS "-Wl,-wrap,accept -Wl,-wrap,read -Wl,-wrap,write"
	)

set_target_properties(
	experiment-client PROPERTIES
	LINK_FLAGS "-Wl,-wrap,connect -Wl,-wrap,read -Wl,-wrap,write"
	)
