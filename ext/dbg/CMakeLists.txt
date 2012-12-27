cmake_minimum_required(VERSION 2.8)

# Try finding the fungo library, which is required by dbg.
set(fungo_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../fungo) # FIXME
if (NOT DEFINED fungo_ROOT)
    message(FATAL_ERROR "fungo_ROOT was not defined")
elseif(NOT EXISTS ${fungo_ROOT})
    message(FATAL_ERROR "fungo could not be found. fungo_ROOT=${fungo_ROOT}")
endif()
include_directories(${fungo_ROOT}/include)
file(GLOB fungo_sources ${fungo_ROOT}/src/*)
add_library(fungo ${fungo_sources})


project(dbg)

# Gather the right source files depending on our platform.
file(GLOB common_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB osx_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/*.cpp)
file(GLOB msvc_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/msvc/*.cpp)
file(GLOB mingw_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/mingw/*.cpp)
if (MSVC)
    set(all_sources ${common_sources} ${msvc_sources})
elseif (MINGW)
    set(all_sources ${common_sources} ${mingw_sources})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   set(all_sources ${common_sources} ${osx_sources})
else()
    message(FATAL_ERROR "Building on your system is not supported right now.")
endif()

# Add the DBG_RECENT_DBGHELP_DLL preprocessor definition if the DBGHELP_PATH
# variable was passed to Cmake. See [bitbucket.org/edd/dbg/wiki/building].
if (DEFINED DBGHELP_PATH)
    add_definitions(-DDBG_RECENT_DBGHELP_DLL=${DBGHELP_PATH})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(dbg SHARED ${all_sources})
target_link_libraries(dbg fungo)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
