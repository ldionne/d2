
add_executable(analyze analyze.cpp)
target_link_libraries(analyze d2_lib)

add_executable(show_graph show_graph.cpp)
target_link_libraries(show_graph d2_lib ${Boost_LIBRARIES})

add_executable(graph_stats graph_stats.cpp)
target_link_libraries(graph_stats d2_lib)

add_executable(d2 d2.cpp)
target_link_libraries(d2 d2_lib ${Boost_LIBRARIES})
