
include_directories(include)

# Create the d2mock shared library.
file(GLOB d2mock_sources src/*.cpp)
add_library(d2mock SHARED ${d2mock_sources})
set_target_properties(d2mock PROPERTIES COMPILE_FLAGS -std=c++11)
add_definitions(-DD2MOCK_DYN_LINK)
target_link_libraries(d2mock d2log     ${Boost_THREAD_LIBRARY}
                             d2support ${Boost_SYSTEM_LIBRARY}
                                       ${Boost_FILESYSTEM_LIBRARY})
