/**
 * This file defines an helper class to perform integration tests.
 */

#ifndef D2_MOCK_INTEGRATION_TEST_HPP
#define D2_MOCK_INTEGRATION_TEST_HPP

#include <d2/detail/config.hpp>
#include <d2/lock_id.hpp>
#include <d2/mock/thread.hpp>
#include <d2/thread_id.hpp>

#include <boost/filesystem.hpp>
#include <boost/move/move.hpp>
#include <boost/operators.hpp>
#include <initializer_list>
#include <iosfwd>
#include <string>
#include <vector>


namespace d2 {
namespace mock {

namespace detail {
struct Streak : boost::equality_comparable<Streak> {
    ThreadId thread_id;
    std::vector<LockId> locks;

    Streak() : thread_id() { }

    template <typename Lock1, typename Lock2, typename ...Locks>
    Streak(thread const& thread, Lock1&& lock1, Lock2&& lock2,
                                                    Locks&& ...locks)
        : thread_id(thread),
          locks{LockId(boost::forward<Lock1>(lock1)),
                LockId(boost::forward<Lock2>(lock2)),
                LockId(boost::forward<Locks>(locks))...}
    { }

    friend bool operator==(Streak const& self, Streak const& other) {
        return self.thread_id == other.thread_id &&
               self.locks == other.locks;
    }

    D2_API friend std::ostream& operator<<(std::ostream&, Streak const&);
};

struct D2_API Deadlock {
    std::vector<Streak> steps;

    Deadlock() { }

    Deadlock(std::initializer_list<Streak> const& streaks)
        : steps(streaks)
    { }

    bool is_equivalent_to(Deadlock other) const;

    D2_API friend std::ostream& operator<<(std::ostream&, Deadlock const&);
};
} // end namespace detail

struct D2_API integration_test {
    integration_test(int argc, char const* argv[], std::string const& file);

    ~integration_test();

    void
    verify_deadlocks(std::initializer_list<detail::Deadlock> const& expected);

private:
    boost::filesystem::path repo_;
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_INTEGRATION_TEST_HPP
