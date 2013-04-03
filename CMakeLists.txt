#=============================================================================
# Setup project
#=============================================================================
project(d2 CXX C) # C required for the test_c_api unit test
cmake_minimum_required(VERSION 2.8)

set(PACKAGE_NAME d2)
set(PACKAGE_VERSION 1.0)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")

# Add path for custom modules
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(D2_IS_NESTED FALSE)
else()
    set(D2_IS_NESTED TRUE)
endif()


#=============================================================================
# Setup CMake options
#=============================================================================
include(CMakeDependentOption)
option(D2_ENABLE_PEDANTIC   "Compile with pedantic enabled." ON)
option(D2_ENABLE_WERROR     "Fail and stop if a warning is triggered." OFF)
option(D2_ENABLE_CXX0X      "Enable -std=c++0x and use of c++0x language features if the compiler supports it." OFF)
option(D2_ENABLE_LIBCXX     "Enable -stdlib=libc++ if the compiler supports it (I think only clang does)." OFF)
option(D2_ENABLE_SHARED     "Build d2 as a shared library." ON)
CMAKE_DEPENDENT_OPTION(
    D2_ENABLE_TESTS "Enable the unit tests. To avoid target name conflicts and cluttering the list of targets, unit tests are disabled when d2 is nested inside another project."
    OFF "D2_IS_NESTED" # OFF when D2_IS_NESTED
    ON                 # ON otherwise
)


#=============================================================================
# Setup warning flags
#=============================================================================
include(ConfigureCompilerAndSystem)

macro(append_if condition lst var)
    if (${condition})
        list(APPEND ${lst} ${var})
    endif()
endmacro()

append_if(D2_HAS_WALL_FLAG                      D2_CXX_WARNING_FLAGS -Wall)
append_if(D2_HAS_WEXTRA_FLAG                    D2_CXX_WARNING_FLAGS -Wextra)
append_if(D2_HAS_W_FLAG                         D2_CXX_WARNING_FLAGS -W)
append_if(D2_HAS_WNO_UNUSED_LOCAL_TYPEDEF_FLAG  D2_CXX_WARNING_FLAGS -Wno-unused-local-typedefs)
append_if(D2_HAS_WNO_UNUSED_PARAMETER_FLAG      D2_CXX_WARNING_FLAGS -Wno-unused-parameter)
append_if(D2_HAS_WWRITE_STRINGS_FLAG            D2_CXX_WARNING_FLAGS -Wwrite-strings)
append_if(D2_HAS_WNO_LONG_LONG_FLAG             D2_CXX_WARNING_FLAGS -Wno-long-long)

if (D2_ENABLE_WERROR)
    append_if(D2_HAS_WERROR_FLAG                D2_CXX_WARNING_FLAGS -Werror)
    append_if(D2_HAS_WX_FLAG                    D2_CXX_WARNING_FLAGS -WX)
endif()

if (D2_ENABLE_PEDANTIC)
    append_if(D2_HAS_PEDANTIC_FLAG              D2_CXX_WARNING_FLAGS -pedantic)
endif()


#=============================================================================
# Setup feature flags
#=============================================================================
# Compile with c++0x or not
if (D2_ENABLE_CXX0X AND D2_HAS_STDCXX0X_FLAG)
    list(APPEND D2_CXX_FEATURE_FLAGS -std=c++0x)
endif()

# Dynamic or static library
if (D2_ENABLE_SHARED)
    list(APPEND D2_CXX_FEATURE_FLAGS -DD2_DYN_LINK)
endif()

# Whether to use libc++
if (D2_ENABLE_LIBCXX AND D2_HAS_STDLIB_LIBCXX_FLAG)
    list(APPEND D2_CXX_FEATURE_FLAGS -stdlib=libc++)
endif()

# This is the only place where add_definitions is called.
add_definitions(
    ${D2_CXX_WARNING_FLAGS}
    ${D2_CXX_FEATURE_FLAGS}
)


#=============================================================================
# Setup the d2 library
#=============================================================================
include_directories(include)
add_subdirectory(src)


#=============================================================================
# Setup the d2tool utility
#=============================================================================
add_subdirectory(d2tool)


#=============================================================================
# Setup tests
#=============================================================================
if (D2_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
