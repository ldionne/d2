/**
 * This file implements the `lockable` class.
 */

#ifndef D2_LOCKABLE_HPP
#define D2_LOCKABLE_HPP

#include <d2/basic_lockable.hpp>
#include <d2/detail/inherit_constructors.hpp>

#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
/**
 * Wrapper over a synchronization object modeling the `Lockable` concept.
 *
 * This wrapper augments the behavior of `basic_lockable` with the following:
 *  - When the `*this` is `try_lock()`ed successfully, `d2` is notified
 *    automatically.
 */
template <typename Lockable, bool recursive = false>
struct lockable : basic_lockable<Lockable, recursive> {
private:
    typedef basic_lockable<Lockable, recursive> Base;

public:
    D2_INHERIT_CONSTRUCTORS(lockable, Base)

    /**
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

//! Mixin version of the `lockable` wrapper.
template <typename Derived, bool recursive = false>
struct lockable_mixin : basic_lockable_mixin<Derived, recursive> {
    bool try_lock() BOOST_NOEXCEPT {
        if (static_cast<Derived*>(this)->try_lock_impl()) {
            this->notify_lock();
            return true;
        }
        return false;
    }
};
} // end namespace d2

namespace boost {
    namespace sync {
        template <typename L, bool recursive>
        class is_lockable<d2::lockable<L, recursive> >
            : public boost::mpl::true_
        { };

        template <typename L>
        class is_recursive_mutex_sur_parolle<d2::lockable<L, true> >
            : public boost::mpl::true_
        { };
    }
}

#endif // !D2_LOCKABLE_HPP
