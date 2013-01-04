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
D2_API basic_mutex sink_lock;
D2_API bool event_logging_enabled = false;
static EventSink* event_sink = NULL;

template <typename Event>
void push_event_impl(Event const& event) {
    BOOST_ASSERT_MSG(event_logging_enabled,
                        "pushing an event while event logging is disabled");
    BOOST_ASSERT_MSG(event_sink != NULL,
                                "logging events in an invalid NULL sink");
    event_sink->write(event);
}

D2_API extern void push_event(AcquireEvent const& event) {
    push_event_impl(event);
}

D2_API extern void push_event(ReleaseEvent const& event) {
    push_event_impl(event);
}

D2_API extern void push_event(StartEvent const& event) {
    push_event_impl(event);
}

D2_API extern void push_event(JoinEvent const& event) {
    push_event_impl(event);
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
