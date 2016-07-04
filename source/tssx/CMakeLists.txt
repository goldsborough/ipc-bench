###########################################################
## CONFIGURATION
###########################################################

set(CMAKE_MACOSX_RPATH 1)

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
	${CMAKE_CURRENT_SOURCE_DIR}/source/shared-memory.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/timeouts.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/string-set.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/selective.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/session-table.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/session.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/free-list.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/bridge.c
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
## LIBRARIES
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib)

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
  vector
  dl
)

# target_link_libraries(tssx-server-ex ${TSSX_DEPENDENCIES})
target_link_libraries(tssx-server ${TSSX_DEPENDENCIES})
target_link_libraries(tssx-client ${TSSX_DEPENDENCIES})

###########################################################
## CONFIGURATION FOR TRY
###########################################################

add_executable(try source/try.c)
target_link_libraries(try tssx-server ipc-bench-common)
