/**
 * This file implements the event logging API in d2/logging.hpp.
 */

#define D2_SOURCE
#include <d2/acquire_event.hpp>
#include <d2/detail/basic_atomic.hpp>
#include <d2/detail/config.hpp>
#include <d2/filesystem_dispatcher.hpp>
#include <d2/join_event.hpp>
#include <d2/logging.hpp>
#include <d2/release_event.hpp>
#include <d2/start_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <string>


namespace d2 {

namespace detail {
static FilesystemDispatcher dispatcher;

D2_API extern void push_acquire(SyncObject const& s, Thread const& t,
                                                     unsigned int ignore) {
    if (is_enabled()) {
        AcquireEvent event(s, t);
        event.info.init_call_stack(ignore + 1); // ignore current frame
        dispatcher.dispatch(event);
    }
}

D2_API extern void push_release(SyncObject const& s, Thread const& t) {
    if (is_enabled())
        dispatcher.dispatch(ReleaseEvent(s, t));
}

D2_API extern void push_start(Thread const& parent, Thread const& child) {
    if (is_enabled())
        dispatcher.dispatch(StartEvent(parent, child));
}

D2_API extern void push_join(Thread const& parent, Thread const& child) {
    if (is_enabled())
        dispatcher.dispatch(JoinEvent(parent, child));
}
} // end namespace detail

D2_API extern bool set_log_repository(std::string const& path) {
    try {
        detail::dispatcher.set_root(path);
    } catch(...) {
        return false;
    }
    return true;
}

namespace {
    static detail::basic_atomic<bool> event_logging_enabled(false);
}
D2_API extern void disable_event_logging() {
    event_logging_enabled = false;
}

D2_API extern void enable_event_logging() {
    event_logging_enabled = true;
}

D2_API extern bool is_enabled() {
    return event_logging_enabled;
}

} // end namespace d2
