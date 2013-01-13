# Add unit tests with optional boost dependencies.
# Unit tests will depend on the `unit' target.
add_custom_target(unit)
function(d2_add_unit_test name sources) # boost components...
    if(TARGET d2log)
        d2_add_executable(${name}_exe ${sources} ${ARGN})
        d2_target_link_libraries(${name}_exe d2log)
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exe)
        add_dependencies(unit ${name}_exe)
    else()
        message("Missing d2log library: ${name} will be unavailable.")
    endif()
endfunction()
d2_add_unit_test(concepts unit/concepts.cpp)
d2_add_unit_test(event_dispatcher unit/sandbox/event_dispatcher.cpp)
d2_add_unit_test(event_traits unit/event_traits.cpp)


# Add unit tests requiring Google test.
function(d2_add_gtest_unit_test name source) # boost components...
    if(${GTEST_FOUND})
        d2_add_unit_test(${name} ${source} ${ARGN})
        d2_target_link_libraries(${name}_exe ${GTEST_BOTH_LIBRARIES})
    else()
        message("Missing Google Test library: ${name} will be unavailable.")
    endif()
endfunction()
d2_add_gtest_unit_test(lock_debug_info unit/detail/lock_debug_info.cpp)
d2_add_gtest_unit_test(mixed_events unit/mixed_events.cpp)
d2_add_gtest_unit_test(repository unit/repository.cpp filesystem system)
d2_add_gtest_unit_test(segmentation_graph unit/segmentation_graph.cpp graph)


# Add unit tests for events.
function(d2_add_event_test name) # boost components...
    d2_add_gtest_unit_test(${name} unit/events/${name}.cpp ${ARGN})
endfunction()
d2_add_event_test(acquire_event)
d2_add_event_test(join_event)
d2_add_event_test(release_event)
d2_add_event_test(segment_hop_event)
d2_add_event_test(start_event)


# Add the d2mock library for integration tests.
if(TARGET d2log)
    add_library(d2mock integration/mock.cpp)
    d2_target_link_libraries(d2mock d2log)
    d2_link_boost(d2mock thread system filesystem)
    if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
        set_target_properties(d2mock PROPERTIES COMPILE_FLAGS -std=c++11)
    endif()
else()
    message("Missing the d2log library: d2mock will be unavailable.")
endif()


# Add integration tests.
add_custom_target(integration)
function(d2_add_integration_test name)
    if(NOT TARGET d2log)
        message("Missing the d2log library: ${name} will be unavailable.")
    elseif(NOT TARGET d2mock)
        message("Missing the d2mock library: ${name} will be unavailable.")
    else()
        d2_add_executable(${name}_exe integration/${name}.cpp)
        d2_target_link_libraries(${name}_exe d2log d2mock)
        add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_exe)
        add_dependencies(integration ${name}_exe)
        if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
            set_target_properties(${name}_exe PROPERTIES COMPILE_FLAGS -std=c++11)
        endif()
    endif()
endfunction()
d2_add_integration_test(miss_unless_transitive_closure)
d2_add_integration_test(simple_ABBA)
d2_add_integration_test(simple_ABBA_noise)
d2_add_integration_test(simple_ABBA_redundant)
d2_add_integration_test(simple_ABBA_redundant_diff_functions)
d2_add_integration_test(simple_ABBA_segmented)
d2_add_integration_test(simple_ABC)
d2_add_integration_test(simple_ABC_segmented)
