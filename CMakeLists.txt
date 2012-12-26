
project(d2)
cmake_minimum_required(VERSION 2.8)
enable_testing()

add_definitions(-Wall -Wextra -pedantic -std=c++98)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_definitions(-stdlib=libc++)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif()
include_directories(include)

# Add my local boost 1.53
set(BOOST_ROOT ../boost-trunk)
set(Boost_NO_SYSTEM_PATHS TRUE)
find_package(Boost 1.53.0 COMPONENTS graph thread system program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(utilities)
