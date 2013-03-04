# Add standalone unit tests with optional boost dependencies.
# Unit tests will depend on the `unit' target.
add_custom_target(unit)
function(d2_add_standalone_unit_test name sources) # boost components...
    d2_add_executable(${name}_exe ${sources} ${ARGN})
    add_test(NAME ${name} COMMAND ${name}_exe)
    add_dependencies(unit ${name}_exe)
endfunction()
d2_add_standalone_unit_test(partition_by_index detail/partition_by_index.cpp)
d2_add_standalone_unit_test(uniquely_identifiable test_uniquely_identifiable.cpp)


# Add unit tests requiring Google test and d2support.
function(d2_add_unit_test name source) # boost components...
    if(NOT ${GTEST_FOUND})
        message("Missing Google Test library: ${name} will be unavailable.")
    elseif(NOT TARGET d2support)
        message("Missing d2support library: ${name} will be unavailable.")
    else()
        d2_add_standalone_unit_test(${name} ${source} ${ARGN})
        target_link_libraries(${name}_exe ${GTEST_BOTH_LIBRARIES} d2support)
    endif()
endfunction()
d2_add_unit_test(filesystem core/test_filesystem.cpp filesystem system)
d2_add_unit_test(lock_debug_info detail/lock_debug_info.cpp)
d2_add_unit_test(mixed_events mixed_events.cpp)
d2_add_unit_test(segmentation_graph segmentation_graph.cpp graph)


# Add unit tests for events.
function(d2_add_event_test name) # boost components...
    d2_add_unit_test(${name} events/${name}.cpp ${ARGN})
endfunction()
d2_add_event_test(acquire_event)
d2_add_event_test(join_event)
d2_add_event_test(recursive_acquire_event)
d2_add_event_test(recursive_release_event)
d2_add_event_test(release_event)
d2_add_event_test(segment_hop_event)
d2_add_event_test(start_event)
