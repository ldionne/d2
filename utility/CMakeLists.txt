
d2_add_executable(d2 d2.cpp graph program_options filesystem system serialization)
target_link_libraries(d2 d2support dbg)
