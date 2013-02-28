# Add the d2support static library.
file(GLOB d2support_sources events/*.cpp detail/*.cpp *.cpp)
add_library(d2support STATIC ${d2support_sources})
d2_target_link_libraries(d2support dbg)

# Add the d2log logging library.
if(${Boost_FILESYSTEM_FOUND} AND ${Boost_SYSTEM_FOUND} AND TARGET d2support)
    file(GLOB d2log_sources api/*.cpp)
    add_library(d2log SHARED ${d2log_sources})
    set_target_properties(d2log PROPERTIES COMPILE_DEFINITIONS D2_DYN_LINK)
    d2_target_link_libraries(d2log d2support dbg)
    d2_link_boost(d2log filesystem system)
else()
    message("Missing Boost.Filesystem and/or Boost.System and/or d2support libraries: the d2log logging library will be unavailable.")
endif()

# Add the d2mock library.
if(TARGET d2log AND TARGET d2support)
    file(GLOB d2mock_sources mock/*.cpp)
    add_library(d2mock SHARED ${d2mock_sources})
    set_target_properties(d2mock PROPERTIES COMPILE_DEFINITIONS D2_DYN_LINK)
    d2_target_link_libraries(d2mock d2log d2support)
    d2_link_boost(d2mock thread system filesystem)
    if((${D2_WE_ARE_ON_CLANG}) OR (${CMAKE_COMPILER_IS_GNUCXX}))
        set_target_properties(d2mock PROPERTIES COMPILE_FLAGS -std=c++11)
    endif()
else()
    message("Missing the d2log and/or d2support library: d2mock will be unavailable.")
endif()
