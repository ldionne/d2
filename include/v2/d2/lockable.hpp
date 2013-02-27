/**
 * This file implements the `lockable` wrapper for the `Lockable` concept.
 */

#ifndef D2_LOCKABLE_HPP
#define D2_LOCKABLE_HPP

#include <d2/basic_lockable.hpp>

#include <boost/config.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace d2 {
template <typename Lockable>
struct lockable : basic_lockable<Lockable> {
#   define D2_BASE_CLASS basic_lockable<Lockable>
#   define D2_DERIVED_CLASS lockable
#   include <d2/detail/inherit_constructors.hpp>

    bool try_lock() BOOST_NOEXCEPT {
        if (basic_lockable<Lockable>::try_lock()) {
            notify_lock();
            return true;
        }
        return false;
    }
};
} // end namespace d2

namespace boost {
    namespace sync {
        template <typename L>
        class is_lockable<d2::lockable<L> >
            : public boost::mpl::true_
        { };
    }
}

#endif // !D2_LOCKABLE_HPP
