# Add the d2support static library.
file(GLOB d2support_sources events/*.cpp detail/*.cpp sandbox/*.cpp)
add_library(d2support STATIC ${d2support_sources})
d2_target_link_libraries(d2support dbg)

# Add the d2log logging library.
if(${Boost_FILESYSTEM_FOUND} AND ${Boost_SYSTEM_FOUND} AND TARGET d2support)
    add_library(d2log SHARED logging.cpp)
    set_target_properties(d2log PROPERTIES COMPILE_DEFINITIONS D2_DYN_LINK)
    d2_target_link_libraries(d2log d2support dbg)
    d2_link_boost(d2log filesystem system)
else()
    message("Missing Boost.Filesystem and/or Boost.System and/or d2support libraries: the d2log logging library will be unavailable.")
endif()
