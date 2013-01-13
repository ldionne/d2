/**
 * This file defines the interface to log events which constitute the trace
 * of an analyzed program.
 */

#ifndef D2_LOGGING_HPP
#define D2_LOGGING_HPP

#include <d2/detail/config.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <string>


namespace d2 {
namespace detail {
    // These methods are not part of the public API.
    D2_API extern void push_acquire(SyncObject const&, Thread const&,
                                                       unsigned int);
    D2_API extern void push_release(SyncObject const&, Thread const&);
    D2_API extern void push_start(Thread const&, Thread const&);
    D2_API extern void push_join(Thread const&, Thread const&);
} // end namespace detail

/**
 * Set the path of the repository into which events are written when logging
 * is enabled. A path must be set before logging may start, i.e. before
 * `enable_event_logging` is called for the first time.
 *
 * The `path` must either
 *  - Point to nothing (no file, no directory, etc..).
 *  - Point to an empty directory.
 * Anything else will make the call fail.
 *
 * @return Whether the operation succeeded. Causes of failure include the
 *         non respect of the preconditions amongst others.
 * @note This operation can be considered atomic.
 */
D2_API extern bool set_log_repository(std::string const& path);

/**
 * Overload for users not using `std::string` or using an implementation of
 * `std::string` that is not ABI comptatible with the implementation used
 * when building the library.
 */
D2_API extern bool set_log_repository(char const* path);

/**
 * Disable the logging of events by the deadlock detection framework.
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already disabled is useless yet harmless.
 */
D2_API extern void disable_event_logging();

/**
 * Enable the logging of events by the deadlock detection framework.
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already enabled is useless yet harmless.
 */
D2_API extern void enable_event_logging();

/**
 * Return whether event logging is currently enabled.
 */
D2_API extern bool is_enabled();

/**
 * Return whether event logging is currently disabled.
 */
inline bool is_disabled() {
    return !is_enabled();
}

/**
 * Notify the deadlock detection system of the acquisition of synchronization
 * object `s` by thread `t`.
 */
template <typename SyncObject, typename Thread>
void notify_acquire(SyncObject const& s, Thread const& t) {
                                                // ignore this frame
    detail::push_acquire(d2::SyncObject(s), d2::Thread(t), 1);
}

/**
 * Notify the deadlock detection system of the release of synchronization
 * object `s` by thread `t`.
 */
template <typename SyncObject, typename Thread>
void notify_release(SyncObject const& s, Thread const& t) {
    detail::push_release(d2::SyncObject(s), d2::Thread(t));
}

/**
 * Notify the deadlock detection system of the start of a new thread `child`
 * initiated by `parent`.
 */
template <typename Thread>
void notify_start(Thread const& parent, Thread const& child) {
    detail::push_start(d2::Thread(parent), d2::Thread(child));
}

/**
 * Notify the deadlock detection system of the join of thread `child` by
 * `parent`.
 */
template <typename Thread>
void notify_join(Thread const& parent, Thread const& child) {
    detail::push_join(d2::Thread(parent), d2::Thread(child));
}

} // end namespace d2

#endif // !D2_LOGGING_HPP
