
# add Google test
find_package(GTest 1.50.0 REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
add_definitions(-DGTEST_HAS_TR1_TUPLE=0)

# Target to build all the unit tests.
add_custom_target(unit)

macro(d2_add_test name) # sources...
    add_executable(${name}_executable ${ARGN})
    target_link_libraries(${name}_executable d2 ${GTEST_BOTH_LIBRARIES})
    add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_executable)
    add_dependencies(unit ${name}_executable)
endmacro()

d2_add_test(simple_ABBA simple_ABBA.cpp)
d2_add_test(bounded_io_sequence bounded_io_sequence.cpp)
d2_add_test(logging logging.cpp)
