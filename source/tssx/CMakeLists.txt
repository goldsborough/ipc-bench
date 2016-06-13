###########################################################
## SOURCES
###########################################################

set(TSSX_COMMON_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/connection.c
	${CMAKE_CURRENT_SOURCE_DIR}/hashtable.c
	${CMAKE_CURRENT_SOURCE_DIR}/overrides.c
	${CMAKE_CURRENT_SOURCE_DIR}/shared_memory.c
	${CMAKE_CURRENT_SOURCE_DIR}/timeouts.c
	)

set(TSSX_SERVER_SOURCES
  ${TSSX_COMMON_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/server-overrides.c
)


set(TSSX_CLIENT_SOURCES
  ${TSSX_COMMON_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/client-overrides.c
)

###########################################################
## TARGETS
###########################################################

add_library(tssx-server STATIC ${TSSX_SERVER_SOURCES})
add_library(tssx-client STATIC ${TSSX_CLIENT_SOURCES})

###########################################################
## COMPILER FLAGS
###########################################################

target_compile_options(tssx-server PUBLIC "-O1")
target_compile_options(tssx-client PUBLIC "-O1")

###########################################################
## CONFIGURATION FOR TRY
###########################################################

add_executable(try try.c)
target_link_libraries(try tssx-server ipc-bench-common)
