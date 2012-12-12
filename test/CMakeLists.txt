

macro(d2_add_test name) # sources...
    add_executable(${name}_executable ${ARGN})
    target_link_libraries(${name}_executable d2)
    add_test(${name} ${CMAKE_CURRENT_BINARY_DIR}/${name}_executable)
endmacro()

d2_add_test(simple_ABBA simple_ABBA.cpp)
