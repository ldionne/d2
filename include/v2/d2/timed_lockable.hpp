/**
 * This file implements the `timed_lockable` wrapper for the `TimedLockable`
 * concept.
 */

#ifndef D2_TIMED_LOCKABLE_HPP
#define D2_TIMED_LOCKABLE_HPP

#include <d2/lockable.hpp>

#include <boost/config.hpp>
#include <boost/move/utility.hpp>


namespace d2 {
template <typename TimedLockable>
struct timed_lockable : lockable<TimedLockable> {
#   define D2_BASE_CLASS lockable<TimedLockable>
#   define D2_DERIVED_CLASS timed_lockable
#   include <d2/detail/inherit_constructors.hpp>

    template <typename Duration>
    bool try_lock_for(BOOST_FWD_REF(Duration) rel_time) BOOST_NOEXCEPT {
        if (lockable<TimedLockable>::try_lock_for(
                boost::forward<Duration>(rel_time))) {
            notify_lock();
            return true;
        }
        return false;
    }

    template <typename TimePoint>
    bool try_lock_until(BOOST_FWD_REF(TimePoint) abs_time) BOOST_NOEXCEPT {
        if (lockable<TimedLockable>::try_lock_until(
                boost::forward<TimePoint>(abs_time))) {
            notify_lock();
            return true;
        }
        return false;
    }
};
} // end namespace d2

#endif // !D2_TIMED_LOCKABLE_HPP
