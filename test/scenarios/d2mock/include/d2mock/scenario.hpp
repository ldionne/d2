/*!
 * @file
 * This file defines the `d2mock::check_scenario` function.
 */

#ifndef D2MOCK_SCENARIO_HPP
#define D2MOCK_SCENARIO_HPP

#include <d2mock/detail/decl.hpp>
#include <d2mock/thread.hpp>

#include <boost/function.hpp>
#include <cstddef>
#include <d2/detail/ut_access.hpp>
#include <vector>


namespace d2mock {
namespace check_scenario_detail {
struct deadlocked_thread {
    template <typename Lock1, typename Lock2, typename ...Locks>
    deadlocked_thread(d2mock::thread& thread,
                        Lock1 const& lock1,
                        Lock2 const& lock2,
                        Locks const& ...locks)
        : thread(thread),
          locks{
                d2::detail::ut_access::d2_unique_id(lock1),
                d2::detail::ut_access::d2_unique_id(lock2),
                d2::detail::ut_access::d2_unique_id(locks)...
            }
    { }

    d2mock::thread& thread;
    std::vector<std::size_t> locks;
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
