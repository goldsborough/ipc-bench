###########################################################
## TARGETS
###########################################################

add_executable(eventfd-uni eventfd-uni.c)
add_executable(eventfd-bi eventfd-bi.c)

###########################################################
## COMMON
###########################################################

target_link_libraries(eventfd-uni ipc-bench-common)
target_link_libraries(eventfd-bi ipc-bench-common)
