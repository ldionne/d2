/**
 * This file implements the `basic_lockable` wrapper for the `BasicLockable`
 * concept.
 */

#ifndef D2_BASIC_LOCKABLE_HPP
#define D2_BASIC_LOCKABLE_HPP

#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
template <typename BasicLockable>
struct basic_lockable : BasicLockable {
#   define D2_BASE_CLASS BasicLockable
#   define D2_DERIVED_CLASS basic_lockable
#   include <d2/detail/inherit_constructors.hpp>

    void lock() {
        BasicLockable::lock();
        notify_lock();
    }

    void unlock() BOOST_NOEXCEPT {
        BasicLockable::unlock();
        notify_unlock();
    }
};
} // end namespace d2

namespace boost {
    namespace sync {
        template <typename L>
        class is_basic_lockable<d2::basic_lockable<L> >
            : public boost::mpl::true_
        { };
    }
}

#endif // !D2_BASIC_LOCKABLE_HPP
