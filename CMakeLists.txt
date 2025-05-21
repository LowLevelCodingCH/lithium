cmake_minimum_required(VERSION 4.0)
project(Lithium)

add_library(lith STATIC
	lith.c
	lith.h
)

set_target_properties(lith PROPERTIES OUTPUT_NAME "lith")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
