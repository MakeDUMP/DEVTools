cmake_minimum_required(VERSION 3.16)

# --[ ZLib ] ------------------------------------------------------------------
# get sources from https://github.com/madler/zlib
project(zlib LANGUAGES C)
# set(CMAKE_VERBOSE_MAKEFILE ON)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/*.c)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "z")
# set_target_properties(${PROJECT_NAME} PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}")
# set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-mmmx -msse -msse2 -msse3 -msse4.1")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "_LARGEFILE64_SOURCE;Z_HAVE_UNISTD_H;ZLIB_CONST")
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}>")

# --[ MicroTAR ] --------------------------------------------------------------
# get sources from https://github.com/rxi/microtar
project(microtar LANGUAGES C)
# set(CMAKE_VERBOSE_MAKEFILE ON)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/*.c)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE 1)
set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}>")

# --[ ZStd ] ------------------------------------------------------------------
# get sources from https://github.com/facebook/zstd
