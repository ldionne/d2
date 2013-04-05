/*!
 * @file
 * Wrapper around the C low-level API to make it more C++ish.
 */

#ifndef D2_API_HPP
#define D2_API_HPP

#include <d2/api.h>

#include <cstddef>
#include <string>


namespace d2 {
/*!
 * Forwards to `d2_set_log_repository`.
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

//! Forwards to `d2_unset_log_repository`.
inline void unset_log_repository() {
    d2_unset_log_repository();
}

//! Forwards to `d2_disable_event_logging`.
inline void disable_event_logging() {
    d2_disable_event_logging();
}

//! Forwards to `d2_enable_event_logging`.
inline void enable_event_logging() {
    d2_enable_event_logging();
}

//! Forwards to `d2_is_enabled`.
inline bool is_enabled() {
    return d2_is_enabled() == 1;
}

//! Forwards to d2_is_disabled`.
inline bool is_disabled() {
    return d2_is_disabled() == 1;
}

//! Forwards to `d2_notify_acquire`.
void notify_acquire(std::size_t thread, std::size_t lock) {
    d2_notify_acquire(thread, lock);
}

//! Forwards to `d2_notify_recursive_acquire`.
void notify_recursive_acquire(std::size_t thread, std::size_t lock) {
    d2_notify_recursive_acquire(thread, lock);
}

//! Forwards to `d2_notify_release`.
void notify_release(std::size_t thread, std::size_t lock) {
    d2_notify_release(thread, lock);
}

//! Forwards to `d2_notify_recursive_release`.
void notify_recursive_release(std::size_t thread, std::size_t lock) {
    d2_notify_recursive_release(thread, lock);
}

//! Forwards to `d2_notify_start`.
void notify_start(std::size_t parent, std::size_t child) {
    d2_notify_start(parent, child);
}

//! Forwards to `d2_notify_join`.
void notify_join(std::size_t parent, std::size_t child) {
    d2_notify_join(parent, child);
}
} // end namespace d2

#endif // !D2_API_HPP
