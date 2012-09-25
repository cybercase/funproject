cmake_minimum_required(VERSION 2.8)

project(funproject)
find_package(CUDA QUIET)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

add_subdirectory(exercises)
add_subdirectory(experiments)

