# Basic macro to add tests with boost dependencies.
macro(d2_add_test name source) # additional boost components...
    foreach(component ${ARGN})
        string(TOUPPER ${component} COMPONENT)
        if(NOT ${Boost_${COMPONENT}_FOUND})
            set(missing "${component}, ${missing}")
        endif()
    endforeach()

    if(missing)
        message("Missing dependencies ${missing}: ${name} will be unavailable.")
    else()
        add_executable(${name}_exe ${source})
        d2_target_link_libraries(${name}_exe d2log)
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exe)

        foreach(component ${ARGN})
            string(TOUPPER ${component} COMPONENT)
            d2_target_link_libraries(${name}_exe ${Boost_${COMPONENT}_LIBRARY})
        endforeach()
    endif()
endmacro()


# Macro to add unit tests. Unit tests will depend on the `unit' target.
add_custom_target(unit)
macro(d2_add_unit_test name source) # additional boost components...
    d2_add_test(${name} ${source} ${ARGN})
    if(TARGET ${name}_exe)
        add_dependencies(unit ${name}_exe)
    endif()
endmacro()
d2_add_unit_test(concepts unit/concepts.cpp)
d2_add_unit_test(event_dispatcher unit/sandbox/event_dispatcher.cpp)
d2_add_unit_test(event_traits unit/event_traits.cpp)


# Add unit tests requiring Google test.
if(NOT ${GTEST_FOUND})
    message("Missing Google test library: some unit tests will be unavailable.")
endif()
macro(d2_add_gtest_unit_test name source) # additional boost components...
    if(${GTEST_FOUND})
        d2_add_unit_test(${name} ${source} ${ARGN})
        d2_target_link_libraries(${name}_exe ${GTEST_BOTH_LIBRARIES})
    endif()
endmacro()
d2_add_gtest_unit_test(lock_debug_info unit/detail/lock_debug_info.cpp)
d2_add_gtest_unit_test(mixed_events unit/mixed_events.cpp)
d2_add_gtest_unit_test(repository unit/repository.cpp filesystem system)
d2_add_gtest_unit_test(segmentation_graph unit/segmentation_graph.cpp graph)


# Add unit tests for events
macro(d2_add_event_test name) # additional boost dependencies...
    d2_add_gtest_unit_test(${name} unit/events/${name}.cpp ${ARGN})
endmacro()
d2_add_event_test(acquire_event)
d2_add_event_test(join_event)
d2_add_event_test(release_event)
d2_add_event_test(segment_hop_event)
d2_add_event_test(start_event)


# Add mock library and integration tests.
if(NOT ${Boost_THREAD_FOUND})
    message("Missing Boost.Thread library: integration tests will be unavailable.")
elseif(NOT ${Boost_SYSTEM_FOUND})
    message("Missing Boost.System library: integration tests will be unavailable.")
elseif(NOT ${Boost_FILESYSTEM_FOUND})
    message("Missing Boost.Filesystem library: integration tests will be unavailable.")
else()
    # Mock library.
    add_library(mock ${CMAKE_CURRENT_SOURCE_DIR}/integration/mock.cpp)
    d2_target_link_libraries(mock d2log ${Boost_THREAD_LIBRARY}
                                        ${Boost_SYSTEM_LIBRARY}
                                        ${Boost_FILESYSTEM_LIBRARY})
    if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
        set_target_properties(mock PROPERTIES COMPILE_FLAGS -std=c++11)
    endif()

    # Integration tests.
    add_custom_target(integration)
    macro(d2_add_integration_test name)
        add_executable(${name}_exe ${CMAKE_CURRENT_SOURCE_DIR}/integration/${name}.cpp)
        d2_target_link_libraries(${name}_exe d2log mock)
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exe)
        add_dependencies(integration ${name}_exe)
        if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
            set_target_properties(${name}_exe PROPERTIES COMPILE_FLAGS -std=c++11)
        endif()
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
