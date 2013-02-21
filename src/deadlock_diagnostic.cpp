/**
 * This file implements the `d2/sandbox/deadlock_diagnostic.hpp` header.
 */

#define D2_SOURCE
#include <d2/deadlock_diagnostic.hpp>

#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <iostream>
#include <string>


namespace d2 {

std::ostream& operator<<(std::ostream& os, AcquireStreak const& self) {
    namespace karma = boost::spirit::karma;
    os << karma::format(
        "thread " << karma::stream << " acquired " << (karma::stream % ", "),
        self.thread, self.locks);
    return os;
}

namespace {
    std::string format_explanation(AcquireStreak const&streak){
        BOOST_ASSERT_MSG(streak.locks.size() >= 2,
            "can't format an acquire streak with less than 2 acquisitions");
        return (boost::format("thread %1% acquires %2% and waits for %3%")
                % streak.thread
                % streak.locks.front()
                % streak.locks.back()
                ).str();
    }
}

std::ostream& operator<<(std::ostream& os, DeadlockDiagnostic const& self) {
    namespace karma = boost::spirit::karma;
    os << karma::format(karma::stream % "\nwhile ", self.steps())

       << "\n\nwhich creates a deadlock if\n"

       << karma::format(karma::string % '\n',
            self.steps() | boost::adaptors::transformed(format_explanation));
    return os;
}

} // end namespace d2
