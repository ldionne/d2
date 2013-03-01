/**
 * This file defines an helper class to perform integration tests.
 */

#ifndef D2MOCK_INTEGRATION_TEST_HPP
#define D2MOCK_INTEGRATION_TEST_HPP

#include <d2mock/detail/decl.hpp>
#include <d2mock/thread.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/move/move.hpp>
#include <boost/operators.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>
#include <initializer_list>
#include <iosfwd>
#include <string>
#include <vector>


namespace d2mock {
namespace integration_test_detail {
struct Streak : boost::equality_comparable<Streak> {
    d2::ThreadId thread_id;
    std::vector<d2::LockId> locks;

    Streak() : thread_id() { }

    template <typename Lock1, typename Lock2, typename ...Locks>
    Streak(thread const& thread, Lock1&& lock1, Lock2&& lock2,
                                                    Locks&& ...locks)
        : thread_id(thread),
          locks{d2::LockId(boost::forward<Lock1>(lock1)),
                d2::LockId(boost::forward<Lock2>(lock2)),
                d2::LockId(boost::forward<Locks>(locks))...}
    { }

    friend bool operator==(Streak const& self, Streak const& other) {
        return self.thread_id == other.thread_id &&
               self.locks == other.locks;
    }

    D2MOCK_DECL friend std::ostream& operator<<(std::ostream&, Streak const&);
};

struct Deadlock {
    std::vector<Streak> steps;

    Deadlock() { }

    Deadlock(std::initializer_list<Streak> const& streaks)
        : steps(streaks)
    { }

    D2MOCK_DECL bool is_equivalent_to(Deadlock other) const;

    D2MOCK_DECL friend std::ostream& operator<<(std::ostream&,Deadlock const&);
};

struct integration_test {
    D2MOCK_DECL integration_test(int argc, char const* argv[],
                                 std::string const& file);

    D2MOCK_DECL ~integration_test();

    D2MOCK_DECL void
    verify_deadlocks(std::initializer_list<Deadlock> const& expected);

private:
    boost::filesystem::path repo_;
};
} // end namespace integration_test_detail

using integration_test_detail::integration_test;
} // end namespace d2mock

#endif // !D2MOCK_INTEGRATION_TEST_HPP
