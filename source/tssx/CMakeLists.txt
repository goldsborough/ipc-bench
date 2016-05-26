###########################################################
## SOURCES
###########################################################

set(TSSX_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/tssx.c
	${CMAKE_CURRENT_SOURCE_DIR}/buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/connection.c
	${CMAKE_CURRENT_SOURCE_DIR}/hashtable.c
	${CMAKE_CURRENT_SOURCE_DIR}/overrides.c
	${CMAKE_CURRENT_SOURCE_DIR}/server-overrides.c
	${CMAKE_CURRENT_SOURCE_DIR}/client-overrides.c
	${CMAKE_CURRENT_SOURCE_DIR}/shared_memory.c
)

###########################################################
## TARGETS
###########################################################

add_library(tssx STATIC ${TSSX_SOURCES})
