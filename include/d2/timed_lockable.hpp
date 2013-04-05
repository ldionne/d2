/*!
 * @file
 * This file implements wrappers for the `TimedLockable` concept.
 */

#ifndef D2_TIMED_LOCKABLE_HPP
#define D2_TIMED_LOCKABLE_HPP

#include <d2/detail/inherit_constructors.hpp>
#include <d2/lockable.hpp>
#include <d2/trackable_sync_object.hpp>

#include <boost/config.hpp>
#include <boost/move/utility.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
/*!
 * Wrapper over a synchronization object modeling the `TimedLockable` concept.
 *
 * This wrapper augments the behavior of `d2::lockable` with the following:
 *  - When any one of `try_lock_for()` or `try_lock_until()` is called and
 *    succeeds, `d2` is notified of the acquisition of `*this`.
 */
template <typename TimedLockable, typename Recursive = non_recursive>
class timed_lockable : public lockable<TimedLockable, Recursive> {
    typedef lockable<TimedLockable, Recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(timed_lockable, Base)

    /*!
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

    /*!
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

//! Shortcut for `d2::timed_lockable<RecursiveTimedLockable, d2::recursive>`.
template <typename RecursiveTimedLockable>
class recursive_timed_lockable
    : public timed_lockable<RecursiveTimedLockable, d2::recursive>
{
    typedef timed_lockable<RecursiveTimedLockable, d2::recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(recursive_timed_lockable, Base)
};


#define D2_I_TIMED_LOCKABLE_MIXIN_CODE(Derived)                             \
    D2_I_LOCKABLE_MIXIN_CODE(Derived)                                       \
    template <typename Duration>                                            \
    bool try_lock_for(BOOST_FWD_REF(Duration) rel_time) BOOST_NOEXCEPT {    \
        if (static_cast<Derived*>(this)->                                   \
                try_lock_for_impl(::boost::forward<Duration>(rel_time))) {  \
            this->notify_lock();                                            \
            return true;                                                    \
        }                                                                   \
        return false;                                                       \
    }                                                                       \
                                                                            \
    template <typename TimePoint>                                           \
    bool try_lock_until(BOOST_FWD_REF(TimePoint) abs_time) BOOST_NOEXCEPT { \
        if (static_cast<Derived*>(this)->                                   \
              try_lock_until_impl(::boost::forward<TimePoint>(abs_time))) { \
            this->notify_lock();                                            \
            return true;                                                    \
        }                                                                   \
        return false;                                                       \
    }                                                                       \
/**/

/*!
 * Mixin augmenting the `d2::lockable_mixin` with `try_lock_for()` and
 * `try_lock_until()` methods forwarding to their `*_impl()` counterparts
 * in the `Derived` class.
 *
 * `d2` is notified iff the `try_lock_for_impl()` or the
 * `try_lock_until_impl()` method succeeds when called.
 *
 * @note The `try_lock_for_impl()` and `try_lock_until_impl()` methods must
 *       both be visible to the base class. Granting friendship to the mixin
 *       may be required.
 *
 * @note The issue regarding the specialization of
 *       `boost::is_recursive_mutex_sur_parolle` applies here like it
 *       applies for `d2::basic_lockable_mixin`.
 */
template <typename Derived, typename Recursive = non_recursive>
class timed_lockable_mixin : private trackable_sync_object<Recursive> {
public:
    D2_I_TIMED_LOCKABLE_MIXIN_CODE(Derived)
};

//! Shortcut for `d2::timed_lockable_mixin<Derived, d2::recursive>`.
template <typename Derived>
class recursive_timed_lockable_mixin
    : private trackable_sync_object<recursive>
{
public:
    D2_I_TIMED_LOCKABLE_MIXIN_CODE(Derived)
};
} // end namespace d2

namespace boost { namespace sync {
    template <typename L>
    class is_recursive_mutex_sur_parolle<d2::timed_lockable<L, d2::recursive> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_recursive_mutex_sur_parolle<d2::recursive_timed_lockable<L> >
        : public boost::mpl::true_
    { };
}} // end namespace boost::sync

#endif // !D2_TIMED_LOCKABLE_HPP
