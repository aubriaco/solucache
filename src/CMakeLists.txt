cmake_minimum_required(VERSION 2.8)

## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "solucache" )

## Set our project name
project(${PROJECT_NAME})

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB SRCS "*.cpp")


## Define the executable
add_executable(${PROJECT_NAME} ${SRCS})

link_directories("${CMAKE_SOURCE_DIR}/../lib/")

target_link_libraries(${PROJECT_NAME} solusek ssl crypto crypto++ pthread)

include_directories(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/../../solusek/solusek/include/" "${CMAKE_SOURCE_DIR}/../include/")
