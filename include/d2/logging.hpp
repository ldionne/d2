/**
 * This file defines the interface to log events which constitute the trace
 * of an analyzed program.
 */

#ifndef D2_LOGGING_HPP
#define D2_LOGGING_HPP

#include <d2/detail/config.hpp>
#include <d2/events.hpp>
#include <d2/types.hpp>

#include <boost/concept_check.hpp>
#include <iostream>
#include <string>
#include <vector>


namespace d2 {
namespace detail {
    extern void D2_DECL push_event_impl(Event const& e);

    // FIXME: We could dispatch cleverly depending on the event type and
    //        improve performances. For example, acquire/release events
    //        could be logged inside thread local storage.
    template <typename Event>
    void push_event(Event const& e) {
        push_event_impl(e);
    }
} // end namespace detail

/**
 * Set the sink to which events are written when logging of events is enabled.
 * A sink must be set before logging may start, i.e. before
 * `enable_event_logging` is called for the first time.
 * @note This operation can be considered atomic.
 * @note `sink` must be a valid pointer.
 * @note `*sink` must be in scope as long as the logging of events is enabled
 *       with that sink.
 * @note We use a pointer to make it explicit that only a reference to `sink`
 *       is taken.
 */
extern void D2_DECL set_event_sink(std::ostream* sink);

/**
 * Disable the logging of events by the deadlock detection framework.
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already disabled is useless yet harmless.
 */
extern void D2_DECL disable_event_logging();

/**
 * Enable the logging of events by the deadlock detection framework.
 * The sink that is used is the same that was set last with `set_event_sink`.
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already enabled is useless yet harmless.
 */
extern void D2_DECL enable_event_logging();

/**
 * Return whether event logging is currently enabled.
 */
extern bool D2_DECL is_enabled();

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
 */
extern std::vector<Event> D2_DECL load_events(std::istream& source);

/**
 * Notify the deadlock detection system of the acquisition of synchronization
 * object `s` by thread `t`.
 */
template <typename SyncObject_, typename Thread_>
void notify_acquire(SyncObject_ const& s, Thread_ const& t) {
    BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<SyncObject_>));
    BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<Thread_>));
    if (is_enabled()) {
        AcquireEvent e((SyncObject(s)), Thread(t));
        e.info.init_call_stack(1); // ignore current frame
        detail::push_event(e);
    }
}

/**
 * Notify the deadlock detection system of the release of synchronization
 * object `s` by thread `t`.
 */
template <typename SyncObject_, typename Thread_>
void notify_release(SyncObject_ const& s, Thread_ const& t) {
    BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<SyncObject_>));
    BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<Thread_>));
    if (is_enabled())
        detail::push_event(ReleaseEvent(SyncObject(s), Thread(t)));
}

/**
 * Notify the deadlock detection system of the start of a new thread `child`
 * initiated by `parent`.
 */
template <typename Thread_>
void notify_start(Thread_ const& parent, Thread_ const& child) {
    BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<Thread_>));
    if (is_enabled())
        detail::push_event(StartEvent(Thread(parent), Thread(child)));
}

/**
 * Notify the deadlock detection system of the join of thread `child` by
 * `parent`.
 */
template <typename Thread_>
void notify_join(Thread_ const& parent, Thread_ const& child) {
    BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<Thread_>));
    if (is_enabled())
        detail::push_event(JoinEvent(Thread(parent), Thread(child)));
}

} // end namespace d2

#endif // !D2_LOGGING_HPP
