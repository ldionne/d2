/**
 * This file implements the `trackable_sync_object` class.
 */

#ifndef D2_TRACKABLE_SYNC_OBJECT_HPP
#define D2_TRACKABLE_SYNC_OBJECT_HPP

#include <d2/api.hpp>
#include <d2/uniquely_identifiable.hpp>

#include <boost/config.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace d2 {
namespace trackable_sync_object_detail {
    //! @internal Class holding an identifier unique across all locks.
    struct unique_id_for_all_locks
        : uniquely_identifiable<unique_id_for_all_locks>
    { };
}

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
class trackable_sync_object {
    static std::size_t thread_unique_id(boost::thread::id const& id) {
        // TODO: Find a better way to do this portably, or at least
        //       trigger an error when it would yield invalid results.
        using boost::hash_value;
        return hash_value(id);
    }

    trackable_sync_object_detail::unique_id_for_all_locks unique_id_;

protected:
    /**
     * Notify `d2` of the acquisition of this synchronization object by the
     * current thread.
     */
    void notify_lock() const BOOST_NOEXCEPT {
        std::size_t const tid = thread_unique_id(boost::this_thread::get_id());
        if (recursive)
            notify_recursive_acquire(tid, unique_id_);
        else
            notify_acquire(tid, unique_id_);
    }

    /**
     * Notify `d2` of the release of this synchronization object by the
     * current thread.
     */
    void notify_unlock() const BOOST_NOEXCEPT {
        std::size_t const tid = thread_unique_id(boost::this_thread::get_id());
        if (recursive)
            notify_recursive_release(tid, unique_id_);
        else
            notify_release(tid, unique_id_);
    }
};
} // end namespace d2

#endif // !D2_TRACKABLE_SYNC_OBJECT_HPP
