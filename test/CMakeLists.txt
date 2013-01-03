# Add standalone unit tests.
add_custom_target(unit)
macro(d2_add_unit_test name)
    add_executable(${name}_exec ${name}.cpp)
    target_link_libraries(${name}_exec d2_lib)
    add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exec)
    add_dependencies(unit ${name}_exec)
endmacro()
d2_add_unit_test(concepts)


# Add unit tests requiring Google test.
if(NOT ${GTEST_FOUND})
    message("Missing Google test library: some unit tests will be unavailable.")
else()
    macro(d2_add_gtest_unit_test name)
        d2_add_unit_test(${name})
        target_link_libraries(${name}_exec ${GTEST_BOTH_LIBRARIES})
    endmacro()
    d2_add_gtest_unit_test(bounded_io_sequence)
    d2_add_gtest_unit_test(logging)
endif()


# Add mock library and integration tests.
if(NOT ${Boost_THREAD_FOUND})
    message("Missing Boost.Thread library: integration tests will be unavailable.")
elseif(NOT ${Boost_SYSTEM_FOUND})
    message("Missing Boost.System library: integration tests will be unavailable.")
else()
    # Mock library.
    add_library(mock ${CMAKE_CURRENT_SOURCE_DIR}/mock.cpp)
    target_link_libraries(mock d2_lib ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})
    if(NOT MSVC)
        set_target_properties(mock PROPERTIES COMPILE_FLAGS -std=c++11)
    endif()

    # Integration tests.
    add_custom_target(integration)
    macro(d2_add_integration_test name)
        add_executable(${name}_exec ${name}.cpp)
        target_link_libraries(${name}_exec d2_lib mock)
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exec)
        add_dependencies(integration ${name}_exec)
        if(NOT MSVC)
            set_target_properties(${name}_exec PROPERTIES COMPILE_FLAGS -std=c++11)
        endif()
        add_custom_command(
            TARGET ${name}_exec POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy
            ${Boost_THREAD_LIBRARY} ${CMAKE_CURRENT_BINARY_DIR})
    endmacro()
    d2_add_integration_test(simple_ABBA)
    d2_add_integration_test(simple_ABBA_noise)
    d2_add_integration_test(simple_ABBA_redundant)
    d2_add_integration_test(simple_ABBA_redundant_diff_functions)
    d2_add_integration_test(simple_ABBA_segmented)

    d2_add_integration_test(simple_ABC)
    d2_add_integration_test(simple_ABC_segmented)

    d2_add_integration_test(miss_unless_transitive_closure)
endif()
