/**
 * This file defines an helper class to perform integration tests.
 */

#ifndef D2_MOCK_INTEGRATION_TEST_HPP
#define D2_MOCK_INTEGRATION_TEST_HPP

#include <d2/detail/config.hpp>
#include <d2/sandbox/deadlock_diagnostic.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

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

    struct Streak : boost::equality_comparable<
                        Streak, sandbox::DeadlockDiagnostic::AcquireStreak
                    >
    {
        Thread thread_id;
        std::vector<SyncObject> locks;

        template <typename Thread, typename ...Locks>
        Streak(Thread const& thread, Locks&& ...locks)
            : thread_id(thread),
              locks{SyncObject(boost::forward<Locks>(locks))...}
        { }

        D2_API friend bool
        operator==(Streak const& self,
                   sandbox::DeadlockDiagnostic::AcquireStreak const& other);
    };

    struct D2_API Deadlock : boost::equality_comparable<
                               Deadlock, sandbox::DeadlockDiagnostic
                            >
    {
        std::vector<Streak> steps;

        Deadlock(std::initializer_list<Streak> const& streaks)
            : steps(streaks)
        { }

        D2_API friend bool
        operator==(Deadlock const& self,
                   sandbox::DeadlockDiagnostic const& other);
    };

    void verify_deadlocks(std::initializer_list<Deadlock> const& expected);

private:
    boost::filesystem::path repo_;
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_INTEGRATION_TEST_HPP
