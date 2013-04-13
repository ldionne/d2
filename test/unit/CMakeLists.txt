#=============================================================================
# Setup unit tests
#=============================================================================
include(GoogleTest)
find_package(Boost 1.53 REQUIRED filesystem system graph)

add_custom_target(unit COMMENT "build all the unit tests")
add_custom_target(check_unit
    DEPENDS unit
    COMMAND ${CMAKE_CTEST_COMMAND}
    COMMENT "build and then run all the unit tests"
)
add_dependencies(check unit)
function(d2_add_unit_test name sources)
    add_executable(${name} EXCLUDE_FROM_ALL ${sources})
    set_property(TARGET ${name}
        APPEND PROPERTY INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIRS}
    )
    # Note: Until d2 stops leaking its implementation headers to its clients,
    #       we have to add dyno to our include path.
    set_property(TARGET ${name}
        APPEND PROPERTY INCLUDE_DIRECTORIES ${d2_SOURCE_DIR}/ext/dyno/include
    )

    target_link_libraries(${name} d2 ${GTEST_BOTH_LIBRARIES} ${ARGN})
    add_test(NAME ${name} COMMAND ${name})
    add_dependencies(unit ${name})
endfunction()

# Shortcuts for boost libraries
set(bfs ${Boost_FILESYSTEM_LIBRARY})
set(bsys ${Boost_SYSTEM_LIBRARY})
set(bgraph ${Boost_GRAPH_LIBRARY})

d2_add_unit_test(test_basic_lockable             test_basic_lockable.cpp ${bsys})
# the -std=c++0x flag will be enabled and it clashes with C.
if (NOT D2_ENABLE_CXX0X)
    d2_add_unit_test(test_c_api                  test_c_api.c)
endif()
d2_add_unit_test(test_cyclic_permutation         detail/test_cyclic_permutation.cpp)
d2_add_unit_test(test_filesystem                 core/test_filesystem.cpp ${bfs} ${bsys})
d2_add_unit_test(test_lockable                   test_lockable.cpp ${bsys})
d2_add_unit_test(test_partition_by_index         detail/partition_by_index.cpp)
d2_add_unit_test(test_segmentation_graph         core/test_segmentation_graph.cpp ${bgraph})
d2_add_unit_test(test_thread_function            test_thread_function.cpp)
d2_add_unit_test(test_thread_lockable_archetypes detail/test_thread_lockable_archetypes.cpp)
d2_add_unit_test(test_tiernan_all_cycles         detail/test_tiernan_all_cycles.cpp ${bgraph})
d2_add_unit_test(test_timed_lockable             test_timed_lockable.cpp ${bsys})
d2_add_unit_test(test_trackable_thread           test_trackable_thread.cpp)
d2_add_unit_test(test_unordered_difference       detail/test_unordered_difference.cpp)
d2_add_unit_test(test_vertex_to_edge_path        detail/test_vertex_to_edge_path.cpp)
