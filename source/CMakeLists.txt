###########################################################
## CONFIGURATION
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-std=c11)
link_libraries(m)

###########################################################
## DEPENDENCIES
###########################################################

find_package(PkgConfig REQUIRED)

## ZMQ

pkg_check_modules(ZMQ libzmq libzeromq)

###########################################################
## SUBDIRECTORIES
###########################################################

# Apple's C standard library already contains the math functions
if(NOT APPLE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lm")
endif()

add_subdirectory(common)
add_subdirectory(tcp)
add_subdirectory(shm)
add_subdirectory(mmap)
add_subdirectory(fifo)
add_subdirectory(pipe)
add_subdirectory(domain)
add_subdirectory(mq)
add_subdirectory(signal)
add_subdirectory(shm-sync)

add_subdirectory(tssx)
add_subdirectory(experiment)

if (NOT APPLE)
	add_subdirectory(eventfd)
endif()

if (ZMQ_FOUND)
	add_subdirectory(zeromq)
endif()
