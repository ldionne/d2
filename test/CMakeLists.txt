# Add unit tests.
if(NOT ${GTEST_FOUND})
    message("Missing Google test library: unit tests will be unavailable.")
else()
    add_custom_target(unit)
    macro(d2_add_unit_test name)
        add_executable(${name}_exec ${name}.cpp)
        target_link_libraries(${name}_exec d2_lib ${GTEST_BOTH_LIBRARIES})
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exec)
        set_tests_properties(${name} PROPERTIES TIMEOUT 60)
        add_dependencies(unit ${name}_exec)
    endmacro()

    d2_add_unit_test(bounded_io_sequence bounded_io_sequence.cpp)
    d2_add_unit_test(logging logging.cpp)
endif()


# Add mock library and integration tests.
if(NOT ${Boost_THREAD_FOUND})
    message("Missing Boost.Thread library: mock library and integration tests will be unavailable.")
elseif(NOT ${Boost_SYSTEM_FOUND})
    message("Missing Boost.System library: mock library and integration tests will be unavailable.")
else()
    # Mock library.
    add_library(mock SHARED ${CMAKE_CURRENT_SOURCE_DIR}/mock.cpp)
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
