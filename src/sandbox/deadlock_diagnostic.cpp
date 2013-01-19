/**
 * This file implements the `d2/sandbox/deadlock_diagnostic.hpp` header.
 */

#define D2_SOURCE
#include <d2/sandbox/deadlock_diagnostic.hpp>

#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/spirit/include/karma.hpp>
#include <iostream>
#include <string>


namespace d2 {
namespace sandbox {

std::string DeadlockDiagnostic::format_step(AcquireStreak const& streak) {
    BOOST_ASSERT_MSG(streak.locks.size() == 2,
        "we should only be supporting 2 locks right now, "
        "update this function otherwise");
    return (boost::format("thread %1% acquired %2%, ..., %3%")
            % streak.thread.thread_id
            % streak.locks[0].lock_id
            % streak.locks[1].lock_id
            ).str();
}

std::string DeadlockDiagnostic::format_explanation(AcquireStreak const&streak){
    BOOST_ASSERT_MSG(streak.locks.size() >= 2,
        "can't format an acquire streak with less than 2 acquisitions");
    return (boost::format("thread %1% acquires %2% and waits for %3%")
            % streak.thread.thread_id
            % streak.locks.front().lock_id
            % streak.locks.back().lock_id
            ).str();
}

std::ostream& operator<<(std::ostream& os, DeadlockDiagnostic const& self) {
    namespace karma = boost::spirit::karma;
    using namespace boost::adaptors;

    os << karma::format(karma::string % "\nwhile "
        , self.steps() | transformed(DeadlockDiagnostic::format_step))

       << "\n\nwhich creates a deadlock if\n"

       << karma::format(karma::string % '\n'
        , self.steps() | transformed(DeadlockDiagnostic::format_explanation));
    return os;
}

} // end namespace sandbox
} // end namespace d2
