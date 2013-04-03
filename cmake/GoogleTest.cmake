#=============================================================================
# Setup GoogleTest. It is provided as a submodule in ext/gtest.
#=============================================================================
set(GTEST_ROOT ${d2_SOURCE_DIR}/ext/gtest)

add_subdirectory(${GTEST_ROOT} ${d2_BINARY_DIR}/ext/gtest EXCLUDE_FROM_ALL)

set(GTEST_INCLUDE_DIRS ${GTEST_ROOT}/include)
set(GTEST_FOUND TRUE)
set(GTEST_LIBRARIES gtest)
set(GTEST_MAIN_LIBRARIES gtest_main)
set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
