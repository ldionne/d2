
add_executable(analyze analyze.cpp)
target_link_libraries(analyze d2)

add_executable(show_graph show_graph.cpp)
target_link_libraries(show_graph d2 ${Boost_LIBRARIES})

add_executable(graph_stats graph_stats.cpp)
target_link_libraries(graph_stats d2)
