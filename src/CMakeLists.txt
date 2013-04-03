#=============================================================================
# Setup the d2 target
#=============================================================================

# First, setup dyno
set(DYNO_ENABLE_CXX0X ${D2_ENABLE_CXX0X} CACHE INTERNAL "Enable c++0x when building dyno, if the compiler supports it.")
set(DYNO_ENABLE_LIBCXX ${D2_ENABLE_LIBCXX} CACHE INTERNAL "Enable -stdlib=libc++ when building dyno, if the compiler supports it.")
set(DYNO_ENABLE_SHARED OFF CACHE INTERNAL "Build dyno as a shared library.")
add_subdirectory(${d2_SOURCE_DIR}/ext/dyno ${d2_BINARY_DIR}/ext/dyno)

# Then, setup lists of additional stuff required by d2
list(APPEND includes ${d2_SOURCE_DIR}/ext/dyno/include)
list(APPEND includes ${d2_SOURCE_DIR}/ext/backward-cpp)
list(APPEND includes ${D2_BOOST_INCLUDE})

list(APPEND libraries ${D2_BOOST_FILESYSTEM_LIB})
list(APPEND libraries ${D2_BOOST_GRAPH_LIB})
list(APPEND libraries ${D2_BOOST_SERIALIZATION_LIB})
list(APPEND libraries ${D2_BOOST_SYSTEM_LIB})
list(APPEND libraries dyno)

list(APPEND definitions BOOST_SPIRIT_USE_PHOENIX_V3)
list(APPEND definitions BOOST_UNORDERED_USE_MOVE)


# Setup d2 itself
file(GLOB_RECURSE D2_SOURCES *.cpp)

# Add all the headers to the project for IDEs.
if (MSVC_IDE OR XCODE)
    file(GLOB_RECURSE D2_HEADERS ${d2_SOURCE_DIR}/include/*)
endif()

if (D2_ENABLE_SHARED)
    add_library(d2 SHARED ${D2_SOURCES} ${D2_HEADERS})
else()
    add_library(d2 STATIC ${D2_SOURCES} ${D2_HEADERS})
endif()

target_link_libraries(d2 ${libraries})
set_property(TARGET d2 APPEND PROPERTY COMPILE_DEFINITIONS ${definitions})
set_property(TARGET d2 APPEND PROPERTY INCLUDE_DIRECTORIES ${includes})


#=============================================================================
# Setup installation target
#=============================================================================
install(TARGETS d2
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    COMPONENT d2
)

install(DIRECTORY ${d2_SOURCE_DIR}/include/
    DESTINATION include
    COMPONENT d2
    FILES_MATCHING PATTERN *.hpp
)
