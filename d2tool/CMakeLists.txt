#=============================================================================
# Setup the d2tool target
#=============================================================================
# Setup lists of additional stuff required by d2tool
# Note: Until d2 stops leaking its implementation headers to its clients,
#       we have to add dyno to our include path. This is a TODO.
list(APPEND includes ${d2_SOURCE_DIR}/ext/dyno/include)
list(APPEND includes ${D2_BOOST_INCLUDE})

list(APPEND libraries d2)
list(APPEND libraries ${D2_BOOST_FILESYSTEM_LIB})
list(APPEND libraries ${D2_BOOST_PROGRAM_OPTIONS_LIB})
list(APPEND libraries ${D2_BOOST_SYSTEM_LIB})

# Setup d2tool itself
add_executable(d2tool main.cpp)
target_link_libraries(d2tool ${libraries})
set_property(TARGET d2tool APPEND PROPERTY INCLUDE_DIRECTORIES ${includes})


#=============================================================================
# Setup installation target
#=============================================================================
install(TARGETS d2tool
    RUNTIME DESTINATION bin
    COMPONENT d2tool
)
