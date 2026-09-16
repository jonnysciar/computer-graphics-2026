cmake_minimum_required(VERSION 3.9)

project(computer-graphics-2026 LANGUAGES CXX)

include(cmake/settings.cmake)

file(GLOB_RECURSE SRCS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HDRS "${CMAKE_SOURCE_DIR}/include/*.hpp")

add_executable(main ${HDRS} ${SRCS})


