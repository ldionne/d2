/**
 * This file defines an helper class to perform integration tests.
 */

#ifndef D2_MOCK_INTEGRATION_TEST_HPP
#define D2_MOCK_INTEGRATION_TEST_HPP

#include <d2/detail/config.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <boost/filesystem.hpp>
#include <boost/move/move.hpp>
#include <boost/operators.hpp>
#include <initializer_list>
#include <string>
#include <vector>


namespace d2 {
namespace mock {

struct D2_API integration_test {
    integration_test(int argc, char const* argv[], std::string const& file);

    ~integration_test();

    struct Streak : boost::equality_comparable<Streak> {
        ThreadId thread_id;
        std::vector<LockId> locks;

        Streak() : thread_id() { }

        template <typename Thread, typename ...Locks>
        Streak(Thread const& thread, Locks&& ...locks)
            : thread_id(thread),
              locks{LockId(boost::forward<Locks>(locks))...}
        { }

        friend bool operator==(Streak const& self, Streak const& other) {
            return self.thread_id == other.thread_id &&
                   self.locks == other.locks;
        }
    };

    struct Deadlock : boost::equality_comparable<Deadlock> {
        std::vector<Streak> steps;

        Deadlock() { }

        Deadlock(std::initializer_list<Streak> const& streaks)
            : steps(streaks)
        { }

        friend bool operator==(Deadlock const& self, Deadlock const& other) {
            return self.steps == other.steps;
        }
    };

    void verify_deadlocks(std::initializer_list<Deadlock> const& expected);

private:
    boost::filesystem::path repo_;
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_INTEGRATION_TEST_HPP
