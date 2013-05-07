/*!
 * @file
 * This file implements wrappers for the `BasicLockable` concept.
 */

#ifndef D2_BASIC_LOCKABLE_HPP
#define D2_BASIC_LOCKABLE_HPP

#include <d2/access.hpp>
#include <d2/detail/inherit_constructors.hpp>
#include <d2/detail/ut_access.hpp>
#include <d2/trackable_sync_object.hpp>

#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
/*!
 * Wrapper over a synchronization object modeling the `BasicLockable` concept.
 *
 * When the object is `lock()`ed or `unlock()`ed, `d2` will be notified
 * automatically. Intended usage of this class goes as follow:
 * @code
 *
 *  namespace impl {
 *      class my_basic_lockable {
 *      public:
 *          void lock() {
 *              // ...
 *          }
 *
 *          void unlock() {
 *              // ...
 *          }
 *      };
 *  } // end namespace impl
 *
 *  typedef d2::basic_lockable<impl::my_basic_lockable> my_basic_lockable;
 *
 * @endcode
 *
 * @tparam BasicLockable The type of the wrapped synchronization object.
 * @tparam Recursive
 *         Whether the synchronization object is recursive. It defaults to
 *         `d2::non_recursive`.
 */
template <typename BasicLockable, typename Recursive = non_recursive>
class basic_lockable
    : public BasicLockable,
      protected trackable_sync_object<Recursive>
{
    friend class detail::ut_access;

public:
    D2_INHERIT_CONSTRUCTORS(basic_lockable, BasicLockable)

    /*!
     * Call the `lock()` method of `BasicLockable` and notify `d2` of the
     * acquisition of `*this`.
     */
    void lock() {
        BasicLockable::lock();
        this->notify_lock();
    }

    /*!
     * Call the `unlock()` method of `BasicLockable` and notify `d2` of the
     * release of `*this`.
     */
    void unlock() BOOST_NOEXCEPT {
        BasicLockable::unlock();
        this->notify_unlock();
    }
};

//! Shortcut for `d2::basic_lockable<RecursiveBasicLockable, d2::recursive>`.
template <typename RecursiveBasicLockable>
class recursive_basic_lockable
    : public basic_lockable<RecursiveBasicLockable, recursive>
{
    typedef basic_lockable<RecursiveBasicLockable, recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(recursive_basic_lockable, Base)
};

/*!
 * @internal
 * Code required to create a mixin for the `BasicLockable` concept.
 * The class must also derive from `d2::trackable_sync_object`.
 */
#define D2_I_BASIC_LOCKABLE_MIXIN_CODE(Derived)                             \
    friend class ::d2::detail::ut_access;                                   \
    void lock() {                                                           \
        ::d2::access::lock_impl(static_cast<Derived&>(*this));              \
        this->notify_lock();                                                \
    }                                                                       \
                                                                            \
    void unlock() BOOST_NOEXCEPT {                                          \
        ::d2::access::unlock_impl(static_cast<Derived&>(*this));            \
        this->notify_unlock();                                              \
    }                                                                       \
/**/

/*!
 * Mixin augmenting its derived class with `d2` tractability.
 *
 * By implementing `lock_impl()` and `unlock_impl()` methods, the derived
 * class automatically gets augmented with `lock()` and `unlock()` methods
 * notifying `d2` after forwarding to the `*_impl()` implementation.
 *
 * Intended usage of this class goes as follow:
 * @code
 *
 *  class my_basic_lockable
 *      : public d2::basic_lockable_mixin<my_basic_lockable>
 *  {
 *      friend class d2::access;
 *
 *      void lock_impl() {
 *          // ...
 *      }
 *
 *      void unlock_impl() {
 *          // ...
 *      }
 *  };
 *
 * @endcode
 *
 * @tparam Derived The derived class to augment with tracking.
 * @tparam Recursive
 *         Whether the tracking should assume a recursive locking policy. It
 *         defaults to `d2::non_recursive`.
 *
 * @note Befriend `d2::access` to grant access to `lock_impl()` and
 *       `unlock_impl()` when they are private.
 *
 * @note The `boost::is_recursive_mutex_sur_parolle` trait will _not_ be
 *       specialized automatically when using this mixin. This differs from
 *       the behavior of using `d2::basic_lockable` and is caused by
 *       implementation issues.
 */
template <typename Derived, typename Recursive = non_recursive>
class basic_lockable_mixin : private trackable_sync_object<Recursive> {
public:
    D2_I_BASIC_LOCKABLE_MIXIN_CODE(Derived)
};

//! Shortcut for `d2::basic_lockable_mixin<Derived, d2::recursive>`.
template <typename Derived>
class recursive_basic_lockable_mixin
    : private trackable_sync_object<recursive>
{
public:
    D2_I_BASIC_LOCKABLE_MIXIN_CODE(Derived)
};
} // end namespace d2

namespace boost { namespace sync {
    template <typename L, typename Recursive>
    class is_basic_lockable<d2::basic_lockable<L, Recursive> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_basic_lockable<d2::recursive_basic_lockable<L> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_recursive_mutex_sur_parolle<d2::basic_lockable<L, d2::recursive> >
        : public boost::mpl::true_
    { };

    template <typename L>
    class is_recursive_mutex_sur_parolle<d2::recursive_basic_lockable<L> >
        : public boost::mpl::true_
    { };
}} // end namespace boost::sync

#endif // !D2_BASIC_LOCKABLE_HPP
