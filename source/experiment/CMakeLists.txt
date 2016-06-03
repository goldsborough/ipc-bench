###########################################################
## TARGETS
###########################################################

add_executable(experiment-client client.c)
add_executable(experiment-server server.c)
add_executable(experiment experiment.c)

###########################################################
## DEPENDENCIES
###########################################################

target_link_libraries(experiment ipc-bench-common)
target_link_libraries(experiment-server ipc-bench-common)
target_link_libraries(experiment-client ipc-bench-common)

target_link_libraries(experiment-client tssx-client)
target_link_libraries(experiment-server tssx-server)

###########################################################
## LINKER FLAGS
###########################################################

set_target_properties(
	experiment-server PROPERTIES
	LINK_FLAGS "-Wl,-wrap,accept -Wl,-wrap,read -Wl,-wrap,write -Wl,-wrap,close"
)

set_target_properties(
	experiment-client PROPERTIES
	LINK_FLAGS "-Wl,-wrap,connect -Wl,-wrap,read -Wl,-wrap,write -Wl,-wrap,close"
)

###########################################################
## COMPILER FLAGS
###########################################################

target_compile_options(experiment-server PUBLIC "-O3")
target_compile_options(experiment-client PUBLIC "-O3")
