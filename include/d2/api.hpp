/*!
 * @file
 * Wrapper around the C API to make it more C++ish.
 */

#ifndef D2_API_HPP
#define D2_API_HPP

#include <d2/api.h>
#include <d2/api/api_detail.hpp>

#include <cstddef>
#include <string>


namespace d2 {
/*!
 * Forwards to `d2_set_log_repository`.
 *
 * @see `d2_set_log_repository`
 *
 * @internal The C-style return type is conserved because we might want to use
 *           error codes in the future.
 */
inline int set_log_repository(char const* path) {
    return d2_set_log_repository(path);
}

inline int set_log_repository(std::string const& path) {
    return set_log_repository(path.c_str());
}

/*!
 * Forwards to `d2_unset_log_repository`.
 *
 * @see `d2_unset_log_repository`
 */
inline void unset_log_repository() {
    d2_unset_log_repository();
}

/*!
 * Forwards to `d2_disable_event_logging`.
 *
 * @see `d2_disable_event_logging`
 */
inline void disable_event_logging() {
    d2_disable_event_logging();
}

/*!
 * Forwards to `d2_enable_event_logging`.
 *
 * @see `d2_enable_event_logging`
 */
inline void enable_event_logging() {
    d2_enable_event_logging();
}

/*!
 * Forwards to `d2_is_enabled`.
 *
 * @see `d2_is_enabled`
 */
inline bool is_enabled() {
    return d2_is_enabled() == 1;
}

/*!
 * Forwards to d2_is_disabled`.
 *
 * @see `d2_is_disabled`
 */
inline bool is_disabled() {
    return d2_is_disabled() == 1;
}

/*!
 * Forwards to `d2_notify_acquire`.
 *
 * Additionally, both objects may define a `unique_id` function that can be
 * found via ADL and that must return an unsigned integral type representing
 * the unique identifier of the object.
 *
 * @see `d2_notify_acquire`
 */
template <typename Thread, typename Lock>
void notify_acquire(Thread const& thread, Lock const& lock) {
    d2_notify_acquire(api_detail::unique_id_impl(thread),
                      api_detail::unique_id_impl(lock));
}

/*!
 * Forwards to `d2_notify_recursive_acquire`.
 *
 * Arguments may support the same as with `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_recursive_acquire`
 */
template <typename Thread, typename Lock>
void notify_recursive_acquire(Thread const& thread, Lock const& lock) {
    d2_notify_recursive_acquire(api_detail::unique_id_impl(thread),
                                api_detail::unique_id_impl(lock));
}

/*!
 * Forwards to `d2_notify_release`.
 *
 * Arguments may support the same as with `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_release`
 */
template <typename Thread, typename Lock>
void notify_release(Thread const& thread, Lock const& lock) {
    d2_notify_release(api_detail::unique_id_impl(thread),
                      api_detail::unique_id_impl(lock));
}

/*!
 * Forwards to `d2_notify_recursive_release`.
 *
 * Arguments may support the same as with `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_recursive_release`
 */
template <typename Thread, typename Lock>
void notify_recursive_release(Thread const& thread, Lock const& lock) {
    d2_notify_recursive_release(api_detail::unique_id_impl(thread),
                                api_detail::unique_id_impl(lock));
}

/*!
 * Forwards to `d2_notify_start`.
 *
 * Arguments may support the same as with `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_start`
 */
template <typename Thread>
void notify_start(Thread const& parent, Thread const& child) {
    d2_notify_start(api_detail::unique_id_impl(parent),
                    api_detail::unique_id_impl(child));
}

/*!
 * Forwards to `d2_notify_join`.
 *
 * Arguments may support the same as with `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_join`
 */
template <typename Thread>
void notify_join(Thread const& parent, Thread const& child) {
    d2_notify_join(api_detail::unique_id_impl(parent),
                   api_detail::unique_id_impl(child));
}

/*!
 * Forwards to `d2_get_lock_id`.
 *
 * @see `d2_get_lock_id`
 */
inline std::size_t get_lock_id() {
    return d2_get_lock_id();
}
} // end namespace d2

#endif // !D2_API_HPP
