cmake_minimum_required(VERSION 2.8)

project(d2)
add_definitions(-Wall -Wextra -pedantic -std=c++98)
include_directories(${d2_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(test)
