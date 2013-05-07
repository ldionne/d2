/*!
 * @file
 * This file implements the `d2::access` class.
 */

#ifndef D2_ACCESS_HPP
#define D2_ACCESS_HPP

#include <boost/config.hpp>
#include <boost/move/utility.hpp>


namespace d2 {
/*!
 * Class used to grant access to the internals of a type to `d2`.
 */
class access {
public:
    template <typename Lock>
    static void lock_impl(Lock& lock) {
        lock.lock_impl();
    }

    template <typename Lock>
    static void unlock_impl(Lock& lock) {
        lock.unlock_impl();
    }

    template <typename Lock>
    static bool try_lock_impl(Lock& lock) BOOST_NOEXCEPT {
        return lock.try_lock_impl();
    }

    template <typename Lock, typename Duration>
    static bool try_lock_for_impl(
                Lock& lock, BOOST_FWD_REF(Duration) rel_time) BOOST_NOEXCEPT {
        return lock.try_lock_for_impl(boost::forward<Duration>(rel_time));
    }

    template <typename Lock, typename TimePoint>
    static bool try_lock_until_impl(
                Lock& lock, BOOST_FWD_REF(TimePoint) abs_time) BOOST_NOEXCEPT {
        return lock.try_lock_until_impl(boost::forward<TimePoint>(abs_time));
    }


    template <typename Thread>
    static void join_impl(Thread& thread) {
        thread.join_impl();
    }

    template <typename Thread>
    static void detach_impl(Thread& thread) {
        thread.detach_impl();
    }
};
} // end namespace d2

#endif // !D2_ACCESS_HPP
