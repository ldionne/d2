/**
 * This file implements the `trackable_sync_object` class.
 */

#ifndef D2_TRACKABLE_SYNC_OBJECT_HPP
#define D2_TRACKABLE_SYNC_OBJECT_HPP

#include <boost/config.hpp>


namespace d2 {
/**
 * Base class providing basic facilities to notify the acquisition and release
 * of synchronization objects to `d2`.
 *
 * Deriving from this class will provide the `Derived` class with the
 * `notify_lock()` and the `notify_unlock()` protected methods. These
 * methods should be called as appropriate to notify `d2` of an acquisition
 * or a release of `*this`.
 *
 * @tparam Derived The type of the wrapped synchronization object.
 * @tparam recursive Whether the synchronization object is recursive.
 */
template <typename Derived, bool recursive>
struct trackable_sync_object {
protected:
    /**
     * Notify `d2` of the acquisition of this synchronization object by the
     * current thread.
     */
    void notify_lock() const BOOST_NOEXCEPT {
        int implement_me;
    }

    /**
     * Notify `d2` of the release of this synchronization object by the
     * current thread.
     */
    void notify_unlock() const BOOST_NOEXCEPT {
        int implement_me;
    }
};
} // end namespace d2

#endif // !D2_TRACKABLE_SYNC_OBJECT_HPP
