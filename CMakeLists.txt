cmake_minimum_required(VERSION 2.8.8)
cmake_policy(VERSION 2.8.8)

project(d2)
enable_testing()


# Default the build type to Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()


# Function linking boost components to a target.
function(d2_link_boost target) # boost components
    foreach(component ${ARGN})
        string(TOUPPER ${component} COMPONENT)
        if(${Boost_${COMPONENT}_FOUND})
            target_link_libraries(${target} ${Boost_${COMPONENT}_LIBRARY})
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
if(${CMAKE_COMPILER_IS_GNUCXX})
    add_definitions(-Wno-unused-local-typedefs)
endif()
if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
    add_definitions(-Wall -Wextra -pedantic)
elseif(MSVC)
    add_definitions(-W3 # -W4 and above is just crazy
                    -D _SCL_SECURE_NO_WARNINGS  # disable non standard MSVC
                    -D _CRT_SECURE_NO_WARNINGS) # warnings
endif()


# Try finding Boost libraries.
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.53.0 OPTIONAL_COMPONENTS graph thread program_options filesystem system serialization)
if(NOT Boost_INCLUDE_DIR)
    message(FATAL_ERROR "Boost include directory was not found. Since Boost is used everywhere in d2, CMake generation will now stop.")
else()
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions(-D BOOST_SPIRIT_USE_PHOENIX_V3
                    -D BOOST_ALL_NO_LIB
                    -D BOOST_ALL_DYN_LINK
                    -D BOOST_UNORDERED_USE_MOVE)
endif()


# Add dbg (we should use find_package, but we don't have a FindFungo.cmake)
add_subdirectory(ext/dbg EXCLUDE_FROM_ALL)
include_directories(ext/dbg/include)

add_subdirectory(ext/dyno EXCLUDE_FROM_ALL)
include_directories(ext/dyno/include)


# Try finding Google test.
find_package(GTest 1.50.0)
if(${GTEST_FOUND})
    include_directories(${GTEST_INCLUDE_DIRS})
    add_definitions(-D GTEST_HAS_TR1_TUPLE=0)
endif()


include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(utility)
