cmake_minimum_required(VERSION 2.8)

project(d2)
enable_testing()

# Macro adding link dependencies to a target and copying these dependencies
# to the target's output directory when done. This is to make sure shared
# libraries are always found.
macro(d2_target_link_libraries target) # libaries...
    target_link_libraries(${target} ${ARGN})
    foreach(library ${ARGN})
        if(EXISTS ${library})
            add_custom_command(
                TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
                ${library} $<TARGET_FILE_DIR:${target}>)
        endif()
    endforeach()
endmacro()


# Determine whether we are on clang.
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    set(D2_WE_ARE_ON_CLANG TRUE)
else()
    set(D2_WE_ARE_ON_CLANG FALSE)
endif()


# Add some custom compiler flags.
if(${D2_WE_ARE_ON_CLANG})
    add_definitions(-stdlib=libc++)
    list(APPEND CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
endif()

if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
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
find_package(Boost 1.53.0 REQUIRED filesystem # required to build d2log
                          OPTIONAL_COMPONENTS graph thread system program_options)
if(NOT Boost_INCLUDE_DIR)
    message(FATAL_ERROR "Boost include directory was not found. Stopping CMake generation.")
else()
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(-DBOOST_SPIRIT_USE_PHOENIX_V3)
endif()


# Add the d2 logging library.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE d2log_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(d2log SHARED ${d2log_sources})
add_definitions(-DD2_DYN_LINK)
d2_target_link_libraries(d2log dbg ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})


# Add the d2 command line utility.
if(NOT ${Boost_GRAPH_FOUND})
    message("Missing Boost.Graph library: d2 command line utility will be unavailable.")
elseif(NOT ${Boost_PROGRAM_OPTIONS_FOUND})
    message("Missing Boost.ProgramOptions library: d2 command line utility will be unavailable.")
else()
    add_executable(d2 ${CMAKE_CURRENT_SOURCE_DIR}/utilities/d2.cpp)
    d2_target_link_libraries(d2 d2log ${Boost_GRAPH_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif()


add_subdirectory(test)
