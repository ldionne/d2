cmake_minimum_required(VERSION 2.8)
enable_testing()

project(d2)
add_definitions(-Wall -Wextra -pedantic -std=c++98)
include_directories(include)

# Add my local boost 1.53
set(BOOST_ROOT /Users/louisdionne/Documents/Ordi/boost_deadlocks)
set(Boost_NO_SYSTEM_PATHS TRUE)
find_package(Boost 1.53.0 COMPONENTS thread system graph REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(utilities)
