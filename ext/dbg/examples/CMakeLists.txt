
add_executable(assert ${CMAKE_CURRENT_SOURCE_DIR}/assert.cpp)
target_link_libraries(assert dbg)

add_executable(note ${CMAKE_CURRENT_SOURCE_DIR}/note.cpp)
target_link_libraries(note dbg)

add_executable(terminate ${CMAKE_CURRENT_SOURCE_DIR}/terminate.cpp)
target_link_libraries(terminate dbg)

add_executable(throw ${CMAKE_CURRENT_SOURCE_DIR}/throw.cpp)
target_link_libraries(throw dbg)

add_executable(traceback ${CMAKE_CURRENT_SOURCE_DIR}/traceback.cpp)
target_link_libraries(traceback dbg)

add_executable(unusual ${CMAKE_CURRENT_SOURCE_DIR}/unusual.cpp)
target_link_libraries(unusual dbg)
