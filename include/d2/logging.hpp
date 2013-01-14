/**
 * Wrapper around the C API to make it more C++ish.
 */

#ifndef D2_LOGGING_HPP
#define D2_LOGGING_HPP

#include <d2/logging.h>

#include <cstddef>
#include <string>


namespace d2 {

/**
 * Forwards to `d2_set_log_repository`.
 *
 * @see `d2_set_log_repository`
 * @internal The C-style return type is conserved because we might want to use
 *           error codes in the future.
 */
inline int set_log_repository(char const* path) {
    return d2_set_log_repository(path);
}

inline int set_log_repository(std::string const& path) {
    return set_log_repository(path.c_str());
}

/**
 * Forwards to `d2_disable_event_logging`.
 *
 * @see `d2_disable_event_logging`
 */
inline void disable_event_logging() {
    d2_disable_event_logging();
}

/**
 * Forwards to `d2_enable_event_logging`.
 *
 * @see `d2_enable_event_logging`
 */
inline void enable_event_logging() {
    d2_enable_event_logging();
}

/**
 * Forwards to `d2_is_enabled`.
 *
 * @see `d2_is_enabled`
 */
inline bool is_enabled() {
    return d2_is_enabled() == 1;
}

/**
 * Forwards to d2_is_disabled`.
 *
 * @see `d2_is_disabled`
 */
inline bool is_disabled() {
    return d2_is_disabled() == 1;
}

/**
 * Forwards to `d2_notify_acquire`. Additionally, both objects may define a
 * `unique_id` function that can be found via ADL and that must return an
 * unsigned integral type representing the unique identifier of the object.
 *
 * @see `d2_notify_acquire`
 */
template <typename Thread, typename Lock>
void notify_acquire(Thread const& thread, Lock const& lock) {
    notify_acquire(unique_id(thread), unique_id(lock));
}

inline void notify_acquire(std::size_t thread, std::size_t lock) {
    d2_notify_acquire(thread, lock);
}

/**
 * Forwards to `d2_notify_release`. Arguments may support the same as with
 * `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_release`
 */
template <typename Thread, typename Lock>
void notify_release(Thread const& thread, Lock const& lock) {
    notify_release(unique_id(thread), unique_id(lock));
}

inline void notify_release(std::size_t thread, std::size_t lock) {
    d2_notify_release(thread, lock);
}

/**
 * Forwards to `d2_notify_start`. Arguments may support the same as with
 * `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_start`
 */
template <typename Thread>
void notify_start(Thread const& parent, Thread const& child) {
    notify_start(unique_id(parent), unique_id(child));
}

inline void notify_start(std::size_t parent, std::size_t child) {
    d2_notify_start(parent, child);
}

/**
 * Forwards to `d2_notify_join`. Arguments may support the same as with
 * `notify_acquire`.
 *
 * @see `notify_acquire`
 * @see `d2_notify_join`
 */
template <typename Thread>
void notify_join(Thread const& parent, Thread const& child) {
    notify_join(unique_id(parent), unique_id(child));
}

inline void notify_join(std::size_t parent, std::size_t child) {
    d2_notify_join(parent, child);
}

} // end namespace d2

#endif // !D2_LOGGING_HPP
