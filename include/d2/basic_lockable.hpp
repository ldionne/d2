/**
 * This file implements the `basic_lockable` class.
 */

#ifndef D2_BASIC_LOCKABLE_HPP
#define D2_BASIC_LOCKABLE_HPP

#include <d2/detail/inherit_constructors.hpp>
#include <d2/trackable_sync_object.hpp>

#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
/**
 * Wrapper over a synchronization object modeling the `BasicLockable` concept.
 *
 * When the object is `lock()`ed or `unlock()`ed, `d2` will be notified
 * automatically. Intended usage of this class goes as follow:
 * @code
 *
 *      namespace impl {
 *          class my_basic_lockable { ... };
 *      }
 *
 *      typedef d2::basic_lockable<impl::my_basic_lockable> my_basic_lockable;
 *
 * @endcode
 *
 * @tparam BasicLockable The type of the wrapped synchronization object.
 * @tparam recursive Whether the synchronization object is recursive.
 */
template <typename BasicLockable, bool recursive = false>
struct basic_lockable : BasicLockable, trackable_sync_object<recursive> {
    D2_INHERIT_CONSTRUCTORS(basic_lockable, BasicLockable)

    /**
     * Call the `lock()` method of `BasicLockable` and notify `d2` of the
     * acquisition of `*this`.
     */
    void lock() {
        BasicLockable::lock();
        this->notify_lock();
    }

    /**
     * Call the `unlock()` method of `BasicLockable` and notify `d2` of the
     * release of `*this`.
     */
    void unlock() BOOST_NOEXCEPT {
        BasicLockable::unlock();
        this->notify_unlock();
    }
};

/**
 * @internal
 * Code required to create a mixin for the `BasicLockable` concept.
 * Additionally, the class must also derive from `trackable_sync_object`.
 */
#define D2_BASIC_LOCKABLE_MIXIN_CODE(Derived)                               \
    void lock() {                                                           \
        static_cast<Derived*>(this)->lock_impl();                           \
        this->notify_lock();                                                \
    }                                                                       \
                                                                            \
    void unlock() BOOST_NOEXCEPT {                                          \
        static_cast<Derived*>(this)->unlock_impl();                         \
        this->notify_unlock();                                              \
    }                                                                       \
/**/

//! Mixin version of the `basic_lockable` wrapper.
template <typename Derived, bool recursive = false>
struct basic_lockable_mixin : trackable_sync_object<recursive> {
    D2_BASIC_LOCKABLE_MIXIN_CODE(Derived)
};
} // end namespace d2

namespace boost {
    namespace sync {
        template <typename L, bool recursive>
        class is_basic_lockable<d2::basic_lockable<L, recursive> >
            : public boost::mpl::true_
        { };

        template <typename L>
        class is_recursive_mutex_sur_parolle<d2::basic_lockable<L, true> >
            : public boost::mpl::true_
        { };
    }
}

#endif // !D2_BASIC_LOCKABLE_HPP
