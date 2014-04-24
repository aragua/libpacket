cmake_minimum_required(VERSION 2.6)
project(packet)

set(CMAKE_C_FLAGS "-Wall -Werror -g -O0")

set(LIBRARY_OUTPUT_PATH lib)
set(EXECUTABLE_OUTPUT_PATH bin)
include_directories(include)

file(
        GLOB_RECURSE
        source_files
        src/*
	include/*
)

file(
        GLOB_RECURSE
        include_files
	include/*.h
)

add_library(
        packet
        SHARED
        ${source_files}
)

add_library(
        packet-static
        STATIC
        ${source_files}
)

add_executable(
        packet_test
        test/packet_test.c
)

target_link_libraries(
        packet_test
        packet
)

add_executable(
        dump
        test/dump.c
)

target_link_libraries(
        dump
        packet
)

#add_executable(
#       bench
#        test/bench.c
#)


install(
	TARGETS packet
	DESTINATION lib
	COMPONENT libraries
)

install(FILES ${include_files}
	DESTINATION include/libpacket
	COMPONENT headers)

install(TARGETS packet_test
	DESTINATION bin
	COMPONENT executables )

set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_NAME "packet")
include(CPack)
