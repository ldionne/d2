/*!
 * @file
 * This file implements wrappers for the `Lockable` concept.
 */

#ifndef D2_LOCKABLE_HPP
#define D2_LOCKABLE_HPP

#include <d2/basic_lockable.hpp>
#include <d2/detail/inherit_constructors.hpp>
#include <d2/trackable_sync_object.hpp>

#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
/*!
 * Wrapper over a synchronization object modeling the `Lockable` concept.
 *
 * This wrapper augments the behavior of `d2::basic_lockable` with the
 * following:
 *  - When `*this` is `try_lock()`ed successfully, `d2` is notified.
 */
template <typename Lockable, typename Recursive = non_recursive>
class lockable : public basic_lockable<Lockable, Recursive> {
    typedef basic_lockable<Lockable, Recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(lockable, Base)

    /*!
     * Call the `try_lock()` method of `Lockable` and notify `d2` of the
     * acquisition of `*this` if and only if the acquisition succeeded.
     *
     * @return Whether the acquisition succeeded.
     */
    bool try_lock() BOOST_NOEXCEPT {
        if (Base::try_lock()) {
            this->notify_lock();
            return true;
        }
        return false;
    }
};

//! Shortcut for `d2::lockable<RecursiveLockable, d2::recursive>`.
template <typename RecursiveLockable>
class recursive_lockable : public lockable<RecursiveLockable, d2::recursive> {
    typedef lockable<RecursiveLockable, d2::recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(recursive_lockable, Base)
};


#define D2_I_LOCKABLE_MIXIN_CODE(Derived)                                   \
    D2_I_BASIC_LOCKABLE_MIXIN_CODE(Derived)                                 \
    bool try_lock() BOOST_NOEXCEPT {                                        \
        if (static_cast<Derived*>(this)->try_lock_impl()) {                 \
            this->notify_lock();                                            \
            return true;                                                    \
        }                                                                   \
        return false;                                                       \
    }                                                                       \
/**/

/*!
 * Mixin augmenting the `d2::basic_lockable_mixin` with a `try_lock()` method
 * forwarding to the `try_lock_impl()` method of the `Derived` class.
 *
 * `d2` is notified iff the `try_lock()`ing succeeds.
 *
 * @note The `try_lock_impl()` method must be visible to the base class.
 *       Granting friendship to the mixin may be required.
 *
 * @note The issue regarding the specialization of
 *       `boost::is_recursive_mutex_sur_parolle` applies here like it
 *       applies for `d2::basic_lockable_mixin`.
 */
template <typename Derived, typename Recursive = non_recursive>
class lockable_mixin : public trackable_sync_object<Recursive> {
public:
    D2_I_LOCKABLE_MIXIN_CODE(Derived)
};

//! Shortcut for `d2::lockable_mixin<Derived, d2::recursive>`.
template <typename Derived>
class recursive_lockable_mixin : public trackable_sync_object<recursive> {
public:
    D2_I_LOCKABLE_MIXIN_CODE(Derived)
};

namespace lockable_detail {
    template <typename First, typename Second>
    struct second { typedef Second type; };
}
} // end namespace d2

namespace boost { namespace sync {
    template <typename L, typename Recursive>
    class is_lockable<d2::lockable<L, Recursive> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_lockable<d2::recursive_lockable<L> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_recursive_mutex_sur_parolle<d2::lockable<L, d2::recursive> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_recursive_mutex_sur_parolle<d2::recursive_lockable<L> >
        : public boost::mpl::true_
    { };
}} // end namespace boost::sync

#endif // !D2_LOCKABLE_HPP
