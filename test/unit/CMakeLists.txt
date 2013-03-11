# Add standalone unit tests with optional boost dependencies.
# Unit tests will depend on the `unit' target.
add_custom_target(unit)
function(d2_add_standalone_unit_test name sources) # boost components...
    d2_add_executable(test_${name} ${sources} ${ARGN})
    add_test(NAME ${name} COMMAND test_${name})
    add_dependencies(unit test_${name})
endfunction()
d2_add_standalone_unit_test(partition_by_index detail/partition_by_index.cpp)
d2_add_standalone_unit_test(uniquely_identifiable test_uniquely_identifiable.cpp)

# Add unit tests requiring Google test and d2.
function(d2_add_unit_test name source) # boost components...
    if(NOT ${GTEST_FOUND})
        message("Missing Google Test library: ${name} will be unavailable.")
    else()
        d2_add_standalone_unit_test(${name} ${source} ${ARGN})
        target_link_libraries(test_${name} ${GTEST_BOTH_LIBRARIES} d2)
    endif()
endfunction()
d2_add_unit_test(all_cycles core/test_all_cycles.cpp graph)
d2_add_unit_test(basic_lockable test_basic_lockable.cpp)
d2_add_unit_test(cyclic_permutation core/test_cyclic_permutation.cpp)
d2_add_unit_test(filesystem core/test_filesystem.cpp filesystem system)
d2_add_unit_test(lock_debug_info detail/lock_debug_info.cpp)
d2_add_unit_test(lockable test_lockable.cpp)
d2_add_unit_test(non_equivalent_cycles core/test_non_equivalent_cycles.cpp graph)
d2_add_unit_test(segmentation_graph core/test_segmentation_graph.cpp graph)
d2_add_unit_test(synchronization_skeleton core/test_synchronization_skeleton.cpp filesystem system)
d2_add_unit_test(thread_lockable_archetypes detail/test_thread_lockable_archetypes.cpp)
d2_add_unit_test(timed_lockable test_timed_lockable.cpp)
