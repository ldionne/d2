# Add standalone unit tests with optional boost dependencies.
# Unit tests will depend on the `unit' target.
add_custom_target(unit)
function(d2_add_standalone_unit_test name sources) # boost components...
    d2_add_executable(${name}_exe ${sources} ${ARGN})
    add_test(NAME ${name} COMMAND ${name}_exe)
    add_dependencies(unit ${name}_exe)
endfunction()
d2_add_standalone_unit_test(concepts unit/concepts.cpp)
d2_add_standalone_unit_test(event_dispatcher unit/sandbox/event_dispatcher.cpp)
d2_add_standalone_unit_test(event_traits unit/event_traits.cpp)

d2_add_standalone_unit_test(c_compiler integration/c_compiler.c)
d2_target_link_libraries(c_compiler_exe d2log)


# Add unit tests requiring Google test and d2log.
function(d2_add_unit_test name source) # boost components...
    if(NOT ${GTEST_FOUND})
        message("Missing Google Test library: ${name} will be unavailable.")
    elseif(NOT TARGET d2log)
        message("Missing d2log library: ${name} will be unavailable.")
    else()
        d2_add_standalone_unit_test(${name} ${source} ${ARGN})
        d2_target_link_libraries(${name}_exe ${GTEST_BOTH_LIBRARIES} d2log)
    endif()
endfunction()
d2_add_unit_test(lock_debug_info unit/detail/lock_debug_info.cpp)
d2_add_unit_test(mixed_events unit/mixed_events.cpp)
d2_add_unit_test(repository unit/repository.cpp filesystem system)
d2_add_unit_test(segmentation_graph unit/segmentation_graph.cpp graph)


# Add unit tests for events.
function(d2_add_event_test name) # boost components...
    d2_add_unit_test(${name} unit/events/${name}.cpp ${ARGN})
endfunction()
d2_add_event_test(acquire_event)
d2_add_event_test(join_event)
d2_add_event_test(recursive_acquire_event)
d2_add_event_test(recursive_release_event)
d2_add_event_test(release_event)
d2_add_event_test(segment_hop_event)
d2_add_event_test(start_event)


# Add the d2mock library for integration tests.
if(TARGET d2log AND TARGET d2support)
    add_library(d2mock STATIC integration/mock.cpp)
    d2_target_link_libraries(d2mock d2log d2support)
    # Note: If the dependencies of d2mock change, don't forget to update
    #       the dependencies for the targets using it.
    d2_link_boost(d2mock thread system filesystem)
    if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
        set_target_properties(d2mock PROPERTIES COMPILE_FLAGS -std=c++11)
    endif()
else()
    message("Missing the d2log and/or d2support library: d2mock will be unavailable.")
endif()


# Add integration tests.
add_custom_target(integration)
function(d2_add_integration_test name)
    if(NOT TARGET d2log)
        message("Missing the d2log library: ${name} will be unavailable.")
    elseif(NOT TARGET d2support)
        message("Missing the d2support library: ${name} will be unavailable.")
    elseif(NOT TARGET d2mock)
        message("Missing the d2mock library: ${name} will be unavailable.")
    else()
        # Note: Since d2mock is a static library, we must propagate its
        #       dependencies to the targets using it.
        d2_add_executable(${name}_exe integration/${name}.cpp thread system filesystem)
        d2_target_link_libraries(${name}_exe d2log d2support d2mock)
        add_test(NAME ${name} COMMAND ${name}_exe)
        add_dependencies(integration ${name}_exe)
        if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
            set_target_properties(${name}_exe PROPERTIES COMPILE_FLAGS -std=c++11)
        endif()
    endif()
endfunction()
d2_add_integration_test(ABBA)
d2_add_integration_test(ABBA_false_middle_gate)
d2_add_integration_test(miss_unless_transitive_closure)
d2_add_integration_test(simple_ABBA_noise)
d2_add_integration_test(simple_ABBA_recursive)
d2_add_integration_test(simple_ABBA_redundant)
d2_add_integration_test(simple_ABBA_redundant_diff_functions)
d2_add_integration_test(simple_ABBA_segmented)
d2_add_integration_test(simple_ABC)
d2_add_integration_test(simple_ABC_segmented)
