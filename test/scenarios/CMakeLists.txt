if (NOT D2_HAS_STDCXX0X_FLAG)
    message(SEND_ERROR "The scenario tests require c++0x support. It looks like your compiler does not support it, so they will be skipped.")
    return()
endif()


#=============================================================================
# Setup the d2mock library
#=============================================================================
find_package(Boost 1.53.0 REQUIRED thread system filesystem)
set(D2MOCK_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
list(APPEND D2MOCK_INCLUDE_DIRS ${d2_SOURCE_DIR}/include)
list(APPEND D2MOCK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/d2mock/include)

set(_d2mock_libraries ${Boost_FILESYSTEM_LIBRARY})
list(APPEND _d2mock_libraries ${Boost_SYSTEM_LIBRARY})
list(APPEND _d2mock_libraries ${Boost_THREAD_LIBRARY})
list(APPEND _d2mock_libraries d2)

set(D2MOCK_REQUIRED_COMPILE_FLAGS -std=c++0x)
set(D2MOCK_REQUIRED_COMPILE_DEFINITIONS D2MOCK_TRACKABLE_SYNC_OBJECT_ACCESS)

# Add all the headers to the project for IDEs.
file(GLOB_RECURSE _d2mock_sources d2mock/src/*.cpp)
if (MSVC_IDE OR XCODE)
    file(GLOB_RECURSE _d2mock_headers d2mock/include/*.hpp)
endif()

add_library(d2mock
    STATIC EXCLUDE_FROM_ALL ${_d2mock_sources} ${_d2mock_headers}
)
target_link_libraries(d2mock ${_d2mock_libraries})

set_property(TARGET d2mock
    APPEND PROPERTY INCLUDE_DIRECTORIES ${D2MOCK_INCLUDE_DIRS}
)
set_property(TARGET d2mock
    APPEND PROPERTY COMPILE_FLAGS ${D2MOCK_REQUIRED_COMPILE_FLAGS}
)
set_property(TARGET d2mock
    APPEND PROPERTY COMPILE_DEFINITIONS ${D2MOCK_REQUIRED_COMPILE_DEFINITIONS}
)


#=============================================================================
# Setup the scenarios
#=============================================================================
add_custom_target(build_scenarios COMMENT "build all the scenario tests")
add_custom_target(check_scenarios
    DEPENDS build_scenarios
    COMMAND ${CMAKE_CTEST_COMMAND}
    COMMENT "build and then run all the scenario tests"
)
add_dependencies(check check_scenarios)
function(d2_add_scenario name sources)
    add_executable(${name} EXCLUDE_FROM_ALL ${sources})
    set_property(TARGET ${name}
        APPEND PROPERTY INCLUDE_DIRECTORIES ${D2MOCK_INCLUDE_DIRS}
    )
    set_property(TARGET ${name}
        APPEND PROPERTY COMPILE_FLAGS ${D2MOCK_REQUIRED_COMPILE_FLAGS}
    )
    set_property(TARGET ${name}
        APPEND PROPERTY COMPILE_DEFINITIONS ${D2MOCK_REQUIRED_COMPILE_DEFINITIONS}
    )
    target_link_libraries(${name} d2mock ${ARGN})
    add_test(NAME ${name} COMMAND ${name})
    add_dependencies(build_scenarios ${name})
endfunction()

file(GLOB all_scenarios RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
foreach(scenario_file ${all_scenarios})
    string(REPLACE ".cpp" "" name ${scenario_file})
    d2_add_scenario(scenario_${name} ${name}.cpp)
endforeach()
