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
	${CMAKE_CURRENT_SOURCE_DIR}/source/hashtable.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/overrides.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/shared_memory.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/timeouts.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/string-set.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/selective.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/vector.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/connection-table.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/free-list.c
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

# Necessary for the preload magic
target_link_libraries(tssx-server dl ipc-bench-common)
target_link_libraries(tssx-client dl ipc-bench-common)

target_compile_options(tssx-server PUBLIC -std=c11)
target_compile_options(tssx-client PUBLIC -std=c11)

###########################################################
## CONFIGURATION FOR TRY
###########################################################

add_executable(try source/try.c)
target_link_libraries(try tssx-server ipc-bench-common)
