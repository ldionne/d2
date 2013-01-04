/**
 * This file implements the event logging API in d2/logging.hpp.
 */

#define D2_SOURCE
#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/config.hpp>
#include <d2/event_sink.hpp>
#include <d2/events.hpp>
#include <d2/logging.hpp>

#include <boost/assert.hpp>
#include <cstddef>


namespace d2 {

namespace detail {
static basic_mutex sink_lock;
static bool event_logging_enabled = false;
static EventSink* event_sink = NULL;

template <typename Event>
void push_event_impl(Event const& event) {
    BOOST_ASSERT_MSG(event_logging_enabled,
                        "pushing an event while event logging is disabled");
    BOOST_ASSERT_MSG(event_sink != NULL,
                                "logging events in an invalid NULL sink");
    event_sink->write(event);
}

D2_API extern void push_acquire(SyncObject const& s, Thread const& t,
                                                     unsigned int ignore) {
    detail::sink_lock.lock();
    if (detail::event_logging_enabled) {
        AcquireEvent event(s, t);
        event.info.init_call_stack(ignore + 1); // ignore current frame
        push_event_impl(event);
    }
    detail::sink_lock.unlock();
}

D2_API extern void push_release(SyncObject const& s, Thread const& t) {
    detail::sink_lock.lock();
    if (detail::event_logging_enabled)
        push_event_impl(ReleaseEvent(s, t));
    detail::sink_lock.unlock();
}

D2_API extern void push_start(Thread const& parent, Thread const& child) {
    detail::sink_lock.lock();
    if (detail::event_logging_enabled)
        push_event_impl(StartEvent(parent, child));
    detail::sink_lock.unlock();
}

D2_API extern void push_join(Thread const& parent, Thread const& child) {
    detail::sink_lock.lock();
    if (detail::event_logging_enabled)
        push_event_impl(JoinEvent(parent, child));
    detail::sink_lock.unlock();
}
} // end namespace detail

D2_API extern void set_event_sink(EventSink* sink) {
    BOOST_ASSERT_MSG(sink != NULL, "setting an invalid NULL sink");
    detail::sink_lock.lock();
    detail::event_sink = sink;
    detail::sink_lock.unlock();
}

D2_API extern void disable_event_logging() {
    detail::sink_lock.lock();
    detail::event_logging_enabled = false;
    detail::sink_lock.unlock();
}

D2_API extern void enable_event_logging() {
    detail::sink_lock.lock();
    detail::event_logging_enabled = true;
    detail::sink_lock.unlock();
}

D2_API extern bool is_enabled() {
    detail::sink_lock.lock();
    bool const enabled = detail::event_logging_enabled;
    detail::sink_lock.unlock();
    return enabled;
}

} // end namespace d2
