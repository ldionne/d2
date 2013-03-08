/**
 * This file defines the `check_scenario` function.
 */

#ifndef D2MOCK_SCENARIO_HPP
#define D2MOCK_SCENARIO_HPP

#include <d2mock/detail/decl.hpp>
#include <d2mock/thread.hpp>

#include <boost/function.hpp>
#include <boost/move/utility.hpp>
#include <d2/core/diagnostic.hpp>
#include <d2/lock_id.hpp>
#include <vector>


namespace d2mock {
namespace check_scenario_detail {
struct deadlocked_thread {
    template <typename Lock1, typename Lock2, typename ...Locks>
    deadlocked_thread(d2mock::thread& thread, BOOST_FWD_REF(Lock1) lock1,
                      BOOST_FWD_REF(Lock2) lock2, BOOST_FWD_REF(Locks) ...locks)
        : thread(thread),
          locks{
                d2::LockId(boost::forward<Lock1>(lock1)),
                d2::LockId(boost::forward<Lock2>(lock2)),
                d2::LockId(boost::forward<Locks>(locks))...
            }
    { }

    d2mock::thread& thread;
    std::vector<d2::LockId> locks;
};

typedef std::vector<deadlocked_thread> potential_deadlock;

D2MOCK_DECL extern int
check_scenario(boost::function<void()> const& test_main,
               int argc, char const* argv[],
               std::vector<potential_deadlock> expected);
} // end namespace check_scenario_detail

using check_scenario_detail::check_scenario;
} // end namespace d2mock

#endif // !D2MOCK_SCENARIO_HPP
