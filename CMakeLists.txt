cmake_minimum_required(VERSION 2.8.8)
cmake_policy(VERSION 2.8.8)

project(d2)
enable_testing()


# Default the build type to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


# Function adding link dependencies to a target and copying these dependencies
# to the target's output directory when done. This is to make sure shared
# libraries are always found.
function(d2_target_link_libraries target) # libaries...
    if(TARGET ${target})
        target_link_libraries(${target} ${ARGN})
        foreach(library ${ARGN})
            if(EXISTS ${library})
                add_custom_command(
                    TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E
                    copy ${library} $<TARGET_FILE_DIR:${target}>
                )
            endif()
        endforeach()
    endif()
endfunction()


# Function linking boost components to a target.
function(d2_link_boost target) # boost components
    foreach(component ${ARGN})
        string(TOUPPER ${component} COMPONENT)
        if(${Boost_${COMPONENT}_FOUND})
            d2_target_link_libraries(${target} ${Boost_${COMPONENT}_LIBRARY})
        endif()
    endforeach()
endfunction()


# Function adding an executable with optional boost dependencies. The
# executable is added only if all the boost dependencies are satisfied. If so,
# all the boost libraries upon which the executable depends are linked into it.
function(d2_add_executable target sources) # boost components...
    foreach(component ${ARGN})
        string(TOUPPER ${component} COMPONENT)
        if(NOT ${Boost_${COMPONENT}_FOUND})
            set(missing "${component}, ${missing}")
        else()
            list(APPEND boost_dependencies ${Boost_${COMPONENT}_LIBRARY})
        endif()
    endforeach()

    if(missing)
        message("Missing dependencies ${missing}: ${target} will be unavailable.")
    else()
        add_executable(${target} ${sources})
        d2_link_boost(${target} ${ARGN})
    endif()
endfunction()


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
    add_definitions(-W3) # -W4 and above is just crazy
    add_definitions(-D_SCL_SECURE_NO_WARNINGS
                    -D_CRT_SECURE_NO_WARNINGS) # disable non standard MSVC warning bullshit
endif()


# Try finding Boost libraries.
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.53.0 OPTIONAL_COMPONENTS graph thread program_options filesystem system)
if(NOT Boost_INCLUDE_DIR)
    message(FATAL_ERROR "Boost include directory was not found. Since Boost is used everywhere in d2, CMake generation will now stop.")
else()
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(-DBOOST_SPIRIT_USE_PHOENIX_V3
                    -DBOOST_ALL_NO_LIB
                    -DBOOST_ALL_DYN_LINK)
endif()


# Add dbg (we should use find_package, but we don't have a FindFungo.cmake)
add_subdirectory(ext/dbg)
include_directories(ext/dbg/include)


# Try finding Google test.
find_package(GTest 1.50.0)
if(${GTEST_FOUND})
    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
endif()


include_directories(include)
add_subdirectory(src)
add_subdirectory(test)


# Add the d2 command line utility.
if(TARGET d2support)
    d2_add_executable(d2 utility/d2.cpp graph program_options filesystem system)
    d2_target_link_libraries(d2 d2support dbg)
else()
    message("Missing d2support library: d2 command line utility will be unavailable.")
endif()
