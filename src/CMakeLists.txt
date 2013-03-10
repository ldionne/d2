# Add the d2 library.
file(GLOB d2_sources *.cpp core/*.cpp detail/*.cpp api/*.cpp)
add_library(d2 SHARED ${d2_sources})
target_link_libraries(d2 dbg dyno ${Boost_FILESYSTEM_LIBRARY}
                                  ${Boost_SYSTEM_LIBRARY}
                                  ${Boost_SERIALIZATION_LIBRARY}
                                  ${Boost_GRAPH_LIBRARY})
set_target_properties(d2 PROPERTIES COMPILE_DEFINITIONS D2_DYN_LINK)
