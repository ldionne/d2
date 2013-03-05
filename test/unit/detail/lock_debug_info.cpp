/**
 * This file contains unit tests for the `LockDebugInfo`
 * and the `StackFrame` classes.
 */

#include "../serialization_test.hpp"
#include <d2/detail/lock_debug_info.hpp>


namespace d2 {
namespace test {

struct StackFrameTest {
    typedef detail::StackFrame value_type;

    static value_type get_random_object() {
        return value_type(1234, "fun0", "file0");
    }
};

struct LockDebugInfoWithoutCallStackTest {
    typedef detail::LockDebugInfo value_type;

    static value_type get_random_object() {
        return value_type();
    }
};

struct LockDebugInfoWithCallStackTest : LockDebugInfoWithoutCallStackTest {
    static value_type get_random_object() {
        value_type info = LockDebugInfoWithoutCallStackTest::get_random_object();
        info.init_call_stack();
        return info;
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(StackFrame, SerializationTest, StackFrameTest);
INSTANTIATE_TYPED_TEST_CASE_P(LockDebugInfoWithoutCallStack, SerializationTest, LockDebugInfoWithoutCallStackTest);
INSTANTIATE_TYPED_TEST_CASE_P(LockDebugInfoWithCallStack, SerializationTest, LockDebugInfoWithCallStackTest);

} // end namespace test
} // end namespace d2
