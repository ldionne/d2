/**
 * This file implements the `timed_lockable` class.
 */

#ifndef D2_TIMED_LOCKABLE_HPP
#define D2_TIMED_LOCKABLE_HPP

#include <d2/detail/inherit_constructors.hpp>
#include <d2/lockable.hpp>

#include <boost/config.hpp>
#include <boost/move/utility.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
/**
 * Wrapper over a synchronization object modeling the `TimedLockable` concept.
 *
 * This wrapper augments the behavior of `lockable` with the following:
 *  - When any one of `try_lock_for()` and `try_lock_until()` is called and
 *    successfully acquires `*this`, `d2` is notified automatically.
 */
template <typename TimedLockable, bool recursive = false>
struct timed_lockable : lockable<TimedLockable, recursive> {
private:
    typedef lockable<TimedLockable, recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(timed_lockable, Base)

    /**
     * Call the `try_lock_for()` method of `TimedLockable` and notify `d2`
     * of the acquisition if and only if it succeeded.
     *
     * @return Whether the acquisition succeeded.
     */
    template <typename Duration>
    bool try_lock_for(BOOST_FWD_REF(Duration) rel_time) BOOST_NOEXCEPT {
        if (Base::try_lock_for(boost::forward<Duration>(rel_time))) {
            this->notify_lock();
            return true;
        }
        return false;
    }

    /**
     * Call the `try_lock_until()` method of `TimedLockable` and notify `d2`
     * of the acquisition if and only if it succeeded.
     *
     * @return Whether the acquisition succeeded.
     */
    template <typename TimePoint>
    bool try_lock_until(BOOST_FWD_REF(TimePoint) abs_time) BOOST_NOEXCEPT {
        if (Base::try_lock_until(boost::forward<TimePoint>(abs_time))) {
            this->notify_lock();
            return true;
        }
        return false;
    }
};
} // end namespace d2

namespace boost {
    namespace sync {
        template <typename L>
        class is_recursive_mutex_sur_parolle<d2::timed_lockable<L, true> >
            : public boost::mpl::true_
        { };
    }
}

#endif // !D2_TIMED_LOCKABLE_HPP
