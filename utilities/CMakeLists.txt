
add_executable(analyze analyze.cpp)
set_target_properties(analyze PROPERTIES COMPILE_FLAGS -std=c++11)
target_link_libraries(analyze d2)

add_executable(show_graph show_graph.cpp)
target_link_libraries(show_graph d2 ${Boost_GRAPH_LIBRARY})
