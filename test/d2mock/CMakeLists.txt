
include_directories(include)

# Create the d2mock shared library.
file(GLOB d2mock_sources src/*.cpp)
add_library(d2mock SHARED ${d2mock_sources})
set_target_properties(d2mock PROPERTIES COMPILE_DEFINITIONS D2MOCK_DYN_LINK
                                        COMPILE_FLAGS -std=c++11)
target_link_libraries(d2mock d2 ${Boost_THREAD_LIBRARY}
                                ${Boost_SYSTEM_LIBRARY}
                                ${Boost_FILESYSTEM_LIBRARY})
