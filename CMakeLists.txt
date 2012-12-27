cmake_minimum_required(VERSION 2.8)

project(d2)
enable_testing()

# Add dbg (we should use find_package, but we don't have a FindFungo.cmake)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/dbg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/dbg/include)

# Add my local boost 1.53
set(BOOST_ROOT ../boost-trunk)
set(Boost_NO_SYSTEM_PATHS TRUE)
find_package(Boost 1.53.0 COMPONENTS graph thread system program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

add_definitions(-Wall -Wextra -pedantic -std=c++98)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_definitions(-stdlib=libc++)
    list(APPEND CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(d2_lib SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp)
target_link_libraries(d2_lib dbg)

add_executable(d2 ${CMAKE_CURRENT_SOURCE_DIR}/utilities/d2.cpp)
target_link_libraries(d2 d2_lib ${Boost_LIBRARIES})

add_subdirectory(test)
