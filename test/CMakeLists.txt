
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Target to build all the unit tests.
add_custom_target(unit)

# Macro to copy all of Boost libraries to a directory after a target has been built.
macro(d2_copy_all_boost_libs_after target directory)
    foreach(lib ${Boost_LIBRARIES})
        if(EXISTS ${lib})
            add_custom_command(
                TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${lib} ${directory}
            )
        endif()
    endforeach()
endmacro()

if(${GTEST_FOUND} AND ${Boost_FOUND})
    macro(d2_add_test name) # sources...
        add_executable(${name}_exec ${ARGN})
        target_link_libraries(${name}_exec d2_lib ${GTEST_BOTH_LIBRARIES}
                                                  ${Boost_LIBRARIES})
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exec)
        set_tests_properties(${name} PROPERTIES TIMEOUT 60)
        add_dependencies(unit ${name}_exec)
        d2_copy_all_boost_libs_after(${name}_exec ${CMAKE_CURRENT_BINARY_DIR})
    endmacro()
else()
    macro(d2_add_test name) # sources...
        message("Missing Boost libraries and/or Google test library. Unit test ${name} won't be available.")
    endmacro()
endif()

d2_add_test(bounded_io_sequence bounded_io_sequence.cpp)
d2_add_test(logging logging.cpp)


macro(d2_add_test_cpp11 name) # sources...
    d2_add_test(${name} ${ARGN})
    if(TARGET ${name}_exec AND NOT MSVC)
        set_target_properties(${name}_exec PROPERTIES COMPILE_FLAGS -std=c++11)
    endif()
endmacro()

d2_add_test_cpp11(simple_ABBA simple_ABBA.cpp)
d2_add_test_cpp11(simple_ABBA_noise simple_ABBA_noise.cpp)
d2_add_test_cpp11(simple_ABBA_redundant simple_ABBA_redundant.cpp)
d2_add_test_cpp11(simple_ABBA_redundant_diff_functions simple_ABBA_redundant_diff_functions.cpp)
d2_add_test_cpp11(simple_ABBA_segmented simple_ABBA_segmented.cpp)

d2_add_test_cpp11(simple_ABC simple_ABC.cpp)
d2_add_test_cpp11(simple_ABC_segmented simple_ABC_segmented.cpp)

d2_add_test_cpp11(miss_unless_transitive_closure miss_unless_transitive_closure.cpp)
