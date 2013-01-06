/**
 * This file contains unit tests for the `LockDebugInfo`
 * and the `StackFrame` classes.
 */

#include <d2/detail/lock_debug_info.hpp>
#include "test_base.hpp"


namespace {
    struct StackFrameAndLockDebugInfoTest : ::testing::Test {
        std::stringstream stream;

        void SetUp() {
            stream.unsetf(std::ios::skipws);
        }
    };
} // end anonymous namespace

TEST_F(StackFrameAndLockDebugInfoTest, save_and_load_a_single_stack_frame) {
    d2::detail::StackFrame saved((void*)0x0, "fun0", "file0");

    stream << saved;
    EXPECT_TRUE(stream && "failed to save the frame");

    d2::detail::StackFrame loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the frame");

    ASSERT_EQ(saved, loaded);
}

TEST_F(StackFrameAndLockDebugInfoTest, save_and_load_several_stack_frames) {
    using namespace boost::assign;
    std::vector<d2::detail::StackFrame> saved;
    saved += d2::detail::StackFrame((void*)0x0, "fun0", "file0"),
             d2::detail::StackFrame((void*)0x1, "fun1", "file1"),
             d2::detail::StackFrame((void*)0x2, "fun2", "file2");

    stream << karma::format(*karma::stream, saved);
    EXPECT_TRUE(stream && "failed to save the frames");

    std::vector<d2::detail::StackFrame> loaded;
    stream >> qi::match(*qi::stream, loaded);
    EXPECT_TRUE(stream && "failed to load the frames");

    ASSERT_EQ(saved, loaded);
}


TEST_F(StackFrameAndLockDebugInfoTest, save_and_load_a_single_info_without_call_stack) {
    d2::detail::LockDebugInfo saved;
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the lock debug info");

    d2::detail::LockDebugInfo loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the lock debug info");

    ASSERT_EQ(saved, loaded);
}

TEST_F(StackFrameAndLockDebugInfoTest, save_and_load_a_single_info_with_call_stack) {
    d2::detail::LockDebugInfo saved;
    saved.init_call_stack();
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the lock debug info");

    d2::detail::LockDebugInfo loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the lock debug info");

    ASSERT_EQ(saved, loaded);
}

TEST_F(StackFrameAndLockDebugInfoTest, save_and_load_several_infos_without_call_stack) {
    std::vector<d2::detail::LockDebugInfo> saved(3);
    stream << karma::format(*karma::stream, saved);
    EXPECT_TRUE(stream && "failed to save the lock debug infos");

    std::vector<d2::detail::LockDebugInfo> loaded;
    stream >> qi::match(*qi::stream, loaded);
    EXPECT_TRUE(stream && "failed to load the lock debug infos");

    ASSERT_EQ(saved, loaded);
}

TEST_F(StackFrameAndLockDebugInfoTest, save_and_load_several_infos_with_call_stack) {
    std::vector<d2::detail::LockDebugInfo> saved(3);
    BOOST_FOREACH(d2::detail::LockDebugInfo& info, saved)
        info.init_call_stack();

    stream << karma::format(*karma::stream, saved);
    EXPECT_TRUE(stream && "failed to save the lock debug infos");

    std::vector<d2::detail::LockDebugInfo> loaded;
    stream >> qi::match(*qi::stream, loaded);
    EXPECT_TRUE(stream && "failed to load the lock debug infos");

    ASSERT_EQ(saved, loaded);
}
