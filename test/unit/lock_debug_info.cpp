/**
 * This file contains unit tests for the `LockDebugInfo`
 * and the `StackFrame` classes.
 */

#include <d2/detail/lock_debug_info.hpp>

#include <algorithm>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <vector>
#include <ios>


namespace karma = boost::spirit::karma;
namespace qi = boost::spirit::qi;
using namespace boost::assign;

namespace std {
    // This is UB. Whatever. We can't say ASSERT_EQ(vector, vector) otherwise.
    template <typename Ostream, typename T, typename A>
    Ostream& operator<<(Ostream& os, vector<T, A> const& v) {
        os << karma::format('(' << karma::stream % ", " << ')', v);
        return os;
    }
} // end namespace std

TEST(stack_frame, save_and_load_a_single_stack_frame) {
    d2::detail::StackFrame saved((void*)0x0, "fun0", "file0");

    std::stringstream stream;
    stream.unsetf(std::ios::skipws);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the frame");

    d2::detail::StackFrame loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the frame");

    ASSERT_EQ(saved, loaded);
}

TEST(stack_frame, save_and_load_several_stack_frames) {
    std::vector<d2::detail::StackFrame> saved;
    saved += d2::detail::StackFrame((void*)0x0, "fun0", "file0"),
             d2::detail::StackFrame((void*)0x1, "fun1", "file1"),
             d2::detail::StackFrame((void*)0x2, "fun2", "file2");

    std::stringstream stream;
    stream.unsetf(std::ios::skipws);
    stream << karma::format(karma::stream % 'n', saved);
    EXPECT_TRUE(stream && "failed to save the frames");

    std::vector<d2::detail::StackFrame> loaded;
    stream >> qi::match(qi::stream % '\n', loaded);
    EXPECT_TRUE(stream && "failed to load the frames");

    ASSERT_EQ(saved, loaded);
}


TEST(lock_debug_info, save_and_load_a_single_info_without_call_stack) {
    d2::detail::LockDebugInfo saved;

    std::stringstream stream;
    stream.unsetf(std::ios::skipws);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the lock debug info");

    d2::detail::LockDebugInfo loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the lock debug info");

    ASSERT_EQ(saved, loaded);
}

TEST(lock_debug_info, save_and_load_a_single_info_with_call_stack) {
    d2::detail::LockDebugInfo saved;
    saved.init_call_stack();

    std::stringstream stream;
    stream.unsetf(std::ios::skipws);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the lock debug info");

    d2::detail::LockDebugInfo loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the lock debug info");

    ASSERT_EQ(saved, loaded);
}

TEST(lock_debug_info, save_and_load_several_infos_without_call_stack) {
    std::vector<d2::detail::LockDebugInfo> saved(3);

    std::stringstream stream;
    stream.unsetf(std::ios::skipws);
    stream << karma::format(karma::stream % '\n', saved);
    EXPECT_TRUE(stream && "failed to save the lock debug infos");

    std::vector<d2::detail::LockDebugInfo> loaded;
    stream >> qi::match(qi::stream % '\n', loaded);
    EXPECT_TRUE(stream && "failed to load the lock debug infos");

    ASSERT_EQ(saved, loaded);
}

TEST(lock_debug_info, save_and_load_several_infos_with_call_stack) {
    std::vector<d2::detail::LockDebugInfo> saved(3);
    BOOST_FOREACH(d2::detail::LockDebugInfo& info, saved)
        info.init_call_stack();

    std::stringstream stream;
    stream.unsetf(std::ios::skipws);
    stream << karma::format(karma::stream % '\n', saved);
    EXPECT_TRUE(stream && "failed to save the lock debug infos");

    std::vector<d2::detail::LockDebugInfo> loaded;
    stream >> qi::match(qi::stream % '\n', loaded);
    EXPECT_TRUE(stream && "failed to load the lock debug infos");

    ASSERT_EQ(saved, loaded);
}
