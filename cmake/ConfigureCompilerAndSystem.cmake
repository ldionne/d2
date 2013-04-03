include(CheckCXXCompilerFlag)
include(CheckLibraryExists)
include(ExternalProject)
include(SetIf)


#=============================================================================
# Check compiler flags
#=============================================================================
check_cxx_compiler_flag(-fno-exceptions             D2_HAS_FNO_EXCEPTIONS_FLAG)
check_cxx_compiler_flag(-fno-rtti                   D2_HAS_FNO_RTTI_FLAG)
check_cxx_compiler_flag(-fPIC                       D2_HAS_FPIC_FLAG)
check_cxx_compiler_flag(-nodefaultlibs              D2_HAS_NODEFAULTLIBS_FLAG)
check_cxx_compiler_flag(-nostdinc++                 D2_HAS_NOSTDINCXX_FLAG)
check_cxx_compiler_flag(-pedantic                   D2_HAS_PEDANTIC_FLAG)
check_cxx_compiler_flag(-std=c++0x                  D2_HAS_STDCXX0X_FLAG)
check_cxx_compiler_flag(-stdlib=libc++              D2_HAS_STDLIB_LIBCXX_FLAG)
check_cxx_compiler_flag(-W                          D2_HAS_W_FLAG)
check_cxx_compiler_flag(-Wall                       D2_HAS_WALL_FLAG)
check_cxx_compiler_flag(-Werror                     D2_HAS_WERROR_FLAG)
check_cxx_compiler_flag(-Wextra                     D2_HAS_WEXTRA_FLAG)
check_cxx_compiler_flag(-Wno-long-long              D2_HAS_WNO_LONG_LONG_FLAG)
check_cxx_compiler_flag(-Wno-unused-local-typedefs  D2_HAS_WNO_UNUSED_LOCAL_TYPEDEFS_FLAG)
check_cxx_compiler_flag(-Wno-unused-parameter       D2_HAS_WNO_UNUSED_PARAMETER_FLAG)
check_cxx_compiler_flag(-Wwrite-strings             D2_HAS_WWRITE_STRINGS_FLAG)


#=============================================================================
# Check support for the Boost libraries
#=============================================================================
find_package(Boost
    OPTIONAL_COMPONENTS thread filesystem system graph program_options serialization
)
set_true_if(Boost_FOUND                 D2_HAS_BOOST_HEADERS)
set_true_if(Boost_FILESYSTEM_FOUND      D2_HAS_BOOST_FILESYSTEM_LIB)
set_true_if(Boost_GRAPH_FOUND           D2_HAS_BOOST_GRAPH_LIB)
set_true_if(Boost_PROGRAM_OPTIONS_FOUND D2_HAS_BOOST_PROGRAM_OPTIONS_LIB)
set_true_if(Boost_SERIALIZATION_FOUND   D2_HAS_BOOST_SERIALIZATION_LIB)
set_true_if(Boost_SYSTEM_FOUND          D2_HAS_BOOST_SYSTEM_LIB)
set_true_if(Boost_THREAD_FOUND          D2_HAS_BOOST_THREAD_LIB)

set_if(Boost_FOUND                      D2_BOOST_VERSION             ${Boost_VERSION})
set_if(D2_HAS_BOOST_HEADERS             D2_BOOST_INCLUDE             ${Boost_INCLUDE_DIRS})
set_if(D2_HAS_BOOST_FILESYSTEM_LIB      D2_BOOST_FILESYSTEM_LIB      ${Boost_FILESYSTEM_LIBRARY})
set_if(D2_HAS_BOOST_GRAPH_LIB           D2_BOOST_GRAPH_LIB           ${Boost_GRAPH_LIBRARY})
set_if(D2_HAS_BOOST_PROGRAM_OPTIONS_LIB D2_BOOST_PROGRAM_OPTIONS_LIB ${Boost_PROGRAM_OPTIONS_LIBRARY})
set_if(D2_HAS_BOOST_SERIALIZATION_LIB   D2_BOOST_SERIALIZATION_LIB   ${Boost_SERIALIZATION_LIBRARY})
set_if(D2_HAS_BOOST_SYSTEM_LIB          D2_BOOST_SYSTEM_LIB          ${Boost_SYSTEM_LIBRARY})
set_if(D2_HAS_BOOST_THREAD_LIB          D2_BOOST_THREAD_LIB          ${Boost_THREAD_LIBRARY})
