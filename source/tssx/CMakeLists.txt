###########################################################
## CONFIGURATION
###########################################################

set(CMAKE_MACOSX_RPATH 1)

add_compile_options(-pthread)

###########################################################
## INCLUDES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

###########################################################
## SOURCES
###########################################################

set(TSSX_COMMON_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/source/buffer.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/connection.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/connection-options.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/hashtable.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/common-overrides.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/socket-overrides.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/poll-overrides.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/select-overrides.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/shared-memory.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/timeouts.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/string-set.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/selective.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/session-table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/session.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/free-list.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/bridge.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/reverse-map.c
	)

set(TSSX_SERVER_SOURCES
  ${TSSX_COMMON_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/server-overrides.c
)


set(TSSX_CLIENT_SOURCES
  ${TSSX_COMMON_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/client-overrides.c
)

###########################################################
## TARGETS
###########################################################

add_library(tssx-server SHARED ${TSSX_SERVER_SOURCES})
add_library(tssx-client SHARED ${TSSX_CLIENT_SOURCES})

###########################################################
## COMPILER FLAGS
###########################################################

set(TSSX_DEPENDENCIES
  ipc-bench-common
  dl
)

target_link_libraries(tssx-server ${TSSX_DEPENDENCIES})
target_link_libraries(tssx-client ${TSSX_DEPENDENCIES})
