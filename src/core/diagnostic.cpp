/**
 * This file implements the `plain_text_explanation` function.
 */

#define D2_SOURCE
#include <d2/core/diagnostic.hpp>

#include <boost/assert.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/spirit/include/karma.hpp>
#include <iterator>
#include <ostream>
#include <string>


namespace karma = boost::spirit::karma;

namespace d2 {
namespace diagnostic_detail {
namespace {
std::string show_thread(deadlocked_thread const& thread) {
    std::string ret;
    karma::generate(std::back_inserter(ret),
        "thread " << karma::stream << " acquired " << (karma::stream % ", "),
        thread.tid, thread.locks);
    return ret;
}

// Note: copying the deadlocked_thread is voluntary so we can modify it inside.
std::string explain_thread(deadlocked_thread thread) {
    BOOST_ASSERT_MSG(thread.locks.size() >= 2,
        "a thread can't be deadlocked if it holds less than two locks");

    std::string ret;
    LockId last_lock = thread.locks.back();
    thread.locks.pop_back();

    karma::generate(std::back_inserter(ret),
        "thread " << karma::stream << " acquires " << (karma::stream % ", ")
                                   << " and waits for " << karma::stream,
        thread.tid, thread.locks, last_lock);
    return ret;
}
} // end anonymous namespace

D2_DECL extern std::ostream&
plain_text_explanation(std::ostream& os, potential_deadlock const& dl) {
    os << karma::format(karma::string % "\nwhile ",
                        dl | boost::adaptors::transformed(show_thread))

       << "\n\nwhich creates a deadlock if\n"

       << karma::format(karma::string % '\n',
                        dl | boost::adaptors::transformed(explain_thread));
    return os;
}
} // end namespace diagnostic_detail
} // end namespace d2
