/*!
 * @file
 * This file implements the `d2::trackable_sync_object` class.
 */

#ifndef D2_TRACKABLE_SYNC_OBJECT_HPP
#define D2_TRACKABLE_SYNC_OBJECT_HPP

#include <d2/api.hpp>
#include <d2/detail/decl.hpp>
#include <d2/uniquely_identifiable.hpp>

#include <boost/config.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <cstddef>


namespace d2 {
namespace trackable_sync_object_detail {
    //! @internal Class holding an identifier unique across all locks.
    struct unique_id_for_all_locks
        : uniquely_identifiable<unique_id_for_all_locks>
    { };

    /*!
     * @internal
     * Return an unsigned integer representing the identifier of the current
     * thread.
     */
    D2_DECL extern std::size_t this_thread_id();
} // end namespace trackable_sync_object_detail

/*!
 * Tag to signal that it is legal for a synchronization object to be acquired
 * recursively by the same thread.
 */
struct recursive;

/*!
 * Tag to signal that it is not legal for a synchronization object to be
 * acquired recursively by the same thread.
 */
struct non_recursive;

/*!
 * Class providing basic facilities to notify the acquisition and the release
 * of synchronization objects to `d2`.
 *
 * Deriving from this class will provide the derived class with
 * `notify_lock()` and `notify_unlock()` protected methods. These methods
 * should be called as appropriate to notify `d2` of an acquisition or a
 * release of `*this`.
 *
 * @tparam Recursive
 *         Tag signaling whether it is legal for a synchronization object
 *         to be acquired recursively by the same thread. It must be one of
 *         `d2::non_recursive` and `d2::recursive`.
 */
template <typename Recursive>
class trackable_sync_object {
    trackable_sync_object_detail::unique_id_for_all_locks unique_id_;

    // No need to evaluate the metafunctions with the current usage.
    typedef boost::is_same<Recursive, recursive> is_recursive;
    typedef boost::is_same<Recursive, non_recursive> is_non_recursive;

    BOOST_MPL_ASSERT((boost::mpl::or_<is_recursive, is_non_recursive>));

protected:
    /*!
     * Notify `d2` of the acquisition of this synchronization object by the
     * current thread.
     */
    void notify_lock() const BOOST_NOEXCEPT {
        std::size_t const tid = trackable_sync_object_detail::this_thread_id();
        if (::d2::trackable_sync_object<Recursive>::is_recursive::value)
            notify_recursive_acquire(tid, unique_id_);
        else
            notify_acquire(tid, unique_id_);
    }

    /*!
     * Notify `d2` of the release of this synchronization object by the
     * current thread.
     */
    void notify_unlock() const BOOST_NOEXCEPT {
        std::size_t const tid = trackable_sync_object_detail::this_thread_id();
        if (::d2::trackable_sync_object<Recursive>::is_recursive::value)
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
