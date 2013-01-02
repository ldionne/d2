cmake_minimum_required(VERSION 2.8)

project(d2)
enable_testing()

# Add some custom compiler flags.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_definitions(-stdlib=libc++)
    list(APPEND CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
endif()

if((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") OR (${CMAKE_COMPILER_IS_GNUCXX}))
    add_definitions(-Wall -Wextra -pedantic -std=c++98)
elseif(MSVC)
    add_definitions(-W4) # -Wall on MSVC yields 1000s of warnings
endif()


# Add dbg (we should use find_package, but we don't have a FindFungo.cmake)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext/dbg)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/dbg/include)


# Try finding Google test.
find_package(GTest 1.50.0)
if(${GTEST_FOUND})
    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
endif()


# Try finding Boost libraries.
find_package(Boost 1.53.0 COMPONENTS graph thread system program_options)
if(${Boost_INCLUDE_DIR} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "Boost include directory was not found. Stopping CMake generation.")
else()
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS})
endif()


# Add the d2 library.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(d2_lib SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp)
add_definitions(-DD2_DYN_LINK)
target_link_libraries(d2_lib dbg)


# Add the d2 command line utility.
if(NOT ${Boost_GRAPH_FOUND})
    message("Missing Boost.Graph library: d2 command line utility will be unavailable.")
elseif(NOT ${Boost_PROGRAM_OPTIONS_FOUND})
    message("Missing Boost.ProgramOptions library: d2 command line utility will be unavailable.")
else()
    add_executable(d2 ${CMAKE_CURRENT_SOURCE_DIR}/utilities/d2.cpp)
    target_link_libraries(d2 d2_lib ${Boost_GRAPH_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif()


add_subdirectory(test)
