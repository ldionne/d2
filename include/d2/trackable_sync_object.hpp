/**
 * This file implements the `trackable_sync_object` class.
 */

#ifndef D2_TRACKABLE_SYNC_OBJECT_HPP
#define D2_TRACKABLE_SYNC_OBJECT_HPP

#include <d2/api.hpp>
#include <d2/detail/decl.hpp>
#include <d2/uniquely_identifiable.hpp>

#include <boost/config.hpp>
#include <cstddef>


namespace d2 {
namespace trackable_sync_object_detail {
    //! @internal Class holding an identifier unique across all locks.
    struct unique_id_for_all_locks
        : uniquely_identifiable<unique_id_for_all_locks>
    { };

    /**
     * @internal
     * Return an unsigned integer representing the identifier of the current
     * thread.
     */
    D2_DECL extern std::size_t this_thread_id();
} // end namespace trackable_sync_object_detail

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
    trackable_sync_object_detail::unique_id_for_all_locks unique_id_;

protected:
    /**
     * Notify `d2` of the acquisition of this synchronization object by the
     * current thread.
     */
    void notify_lock() const BOOST_NOEXCEPT {
        std::size_t const tid = trackable_sync_object_detail::this_thread_id();
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
        std::size_t const tid = trackable_sync_object_detail::this_thread_id();
        if (recursive)
            notify_recursive_release(tid, unique_id_);
        else
            notify_release(tid, unique_id_);
    }

// This is kind of a hack, but we need to access the unique id of locks in
// the test scenarios in order to validate the results of the analysis.
// When the macro is defined, we can call `unique_id(obj)` on a
// `trackable_sync_object` to retrieve its unique identifier.
#ifdef D2MOCK_TRACKABLE_SYNC_OBJECT_ACCESS
public:
    friend std::size_t unique_id(trackable_sync_object const& self) {
        return unique_id(self.unique_id_);
    }
#endif
};
} // end namespace d2

#endif // !D2_TRACKABLE_SYNC_OBJECT_HPP
