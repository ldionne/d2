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
 */
template <typename BasicLockable>
struct basic_lockable
    : BasicLockable, trackable_sync_object<basic_lockable<BasicLockable> >
{
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
