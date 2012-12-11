/**
 * This file implements the event logging.
 */

#include <d2/detail/support.hpp>
#include <d2/events.hpp>

#include <boost/assert.hpp>
#include <cstddef>
#include <ostream>


namespace d2 {

static detail::basic_mutex sink_lock;
static bool is_enabled = false;
static std::ostream* event_sink = NULL;

namespace detail {
    extern void push_event_impl(event const& e) {
        sink_lock.lock();
        if (is_enabled) {
            BOOST_ASSERT_MSG(event_sink != NULL,
                                    "logging events in an invalid NULL sink");
            *event_sink << e << '\n';
        }
        sink_lock.unlock();
    }
}

extern void set_event_sink(std::ostream* sink) {
    BOOST_ASSERT_MSG(sink != NULL, "setting an invalid NULL sink");
    sink_lock.lock();
    event_sink = sink;
    sink_lock.unlock();
}

extern void disable_event_logging() {
    sink_lock.lock();
    is_enabled = false;
    sink_lock.unlock();
}

extern void enable_event_logging() {
    sink_lock.lock();
    is_enabled = true;
    sink_lock.unlock();
}

} // end namespace d2
