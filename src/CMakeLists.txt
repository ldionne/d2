# Add the d2support static library.
file(GLOB d2support_sources events/*.cpp detail/*.cpp *.cpp)
add_library(d2support STATIC ${d2support_sources})
target_link_libraries(d2support dbg)

# Add the d2log logging library.
if(${Boost_FILESYSTEM_FOUND} AND ${Boost_SYSTEM_FOUND} AND TARGET d2support)
    file(GLOB d2log_sources api/*.cpp)
    add_library(d2log SHARED ${d2log_sources})
    set_target_properties(d2log PROPERTIES COMPILE_DEFINITIONS D2_DYN_LINK)
    target_link_libraries(d2log d2support dbg ${Boost_FILESYSTEM_LIBRARY}
                                              ${Boost_SYSTEM_LIBRARY})
else()
    message("Missing Boost.Filesystem and/or Boost.System and/or d2support libraries: the d2log logging library will be unavailable.")
endif()
