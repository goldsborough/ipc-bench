###########################################################
## CONFIGURATION
###########################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-g -O3 -Wall -Werror)

# Apple's C standard library already contains the math functions
if(APPLE)
	add_compile_options(-std=c11)
else()
	add_compile_options(-std=gnu11)
	link_libraries(m)
endif()

link_libraries(pthread)

###########################################################
## DEPENDENCIES
###########################################################

find_package(PkgConfig REQUIRED)

## ZMQ

pkg_check_modules(ZMQ libzmq)

###########################################################
## SUBDIRECTORIES
###########################################################

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

if (NOT APPLE)
	add_subdirectory(eventfd)
endif()

if (ZMQ_FOUND)
	add_subdirectory(zeromq)
endif()
