/**
 * This file defines the interface to log events which constitute the trace
 * of an analyzed program.
 */

#ifndef D2_LOGGING_HPP
#define D2_LOGGING_HPP

#include <d2/detail/config.hpp>
#include <d2/event_sink.hpp>
#include <d2/events.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <istream>
#include <vector>


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
 * Set the sink to which events are written when logging of events is enabled.
 * A sink must be set before logging may start, i.e. before
 * `enable_event_logging` is called for the first time.
 * @note This operation can be considered atomic.
 * @note `sink` must be a pointer to a valid sink.
 * @note `*sink` must be in scope as long as the logging of events is enabled
 *       with that sink.
 */
D2_API extern void set_event_sink(EventSink* sink);

/**
 * Disable the logging of events by the deadlock detection framework.
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already disabled is useless yet harmless.
 */
D2_API extern void disable_event_logging();

/**
 * Enable the logging of events by the deadlock detection framework.
 * The sink that is used is the same that was set last with `set_event_sink`.
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
 * Load and return the events contained in `source` as a vector of events. The
 * source must have been created by the logging framework to ensure it can be
 * read correctly.
 * @todo Remove this from the public logging api.
 * @todo Use a generalization of `EventSink` instead of `std::istream`.
 */
D2_API extern std::vector<Event> load_events(std::istream& source);

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
