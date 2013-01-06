/**
 * This file implements the event logging API in d2/logging.hpp.
 */

#define D2_SOURCE
#include <d2/detail/basic_atomic.hpp>
#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/config.hpp>
#include <d2/events.hpp>
#include <d2/filesystem_dispatcher.hpp>
#include <d2/logging.hpp>
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

static basic_mutex segment_lock;
static Segment current_segment; // initialized to the initial segment value
static boost::unordered_map<Thread, Segment> segment_of;

namespace {
    template <typename Value, typename Container>
    bool contains(Value const& v, Container const& c) {
        return c.find(v) != c.end();
    }
}

D2_API extern void push_start(Thread const& parent, Thread const& child) {
    if (is_enabled()) {
        segment_lock.lock();
        BOOST_ASSERT_MSG(parent != child, "thread starting itself");
        BOOST_ASSERT_MSG(segment_of.empty() || contains(parent, segment_of),
        "starting a thread from another thread that has not been created yet");
        // segment_of[parent] will be the initial segment value on the very
        // first call, which is the same as current_segment. so this means
        // two things:
        //  - parent_segment will be the initial segment value on the very
        //    first call, and the segment of `parent` on subsequent calls,
        //    which is fine.
        //  - we must PREincrement the current_segment so it is distinct from
        //    the initial value.
        Segment parent_segment = segment_of[parent];
        Segment new_parent_segment = ++current_segment;
        Segment child_segment = ++current_segment;
        segment_of[child] = child_segment;
        segment_of[parent] = new_parent_segment;
        segment_lock.unlock();

        dispatcher.dispatch(StartEvent(parent_segment, new_parent_segment,
                                                       child_segment));
        dispatcher.dispatch(SegmentHopEvent(parent, new_parent_segment));
        dispatcher.dispatch(SegmentHopEvent(child, child_segment));
    }
}

D2_API extern void push_join(Thread const& parent, Thread const& child) {
    if (is_enabled()) {
        segment_lock.lock();
        BOOST_ASSERT_MSG(parent != child, "thread joining itself");
        BOOST_ASSERT_MSG(contains(parent, segment_of),
        "joining a thread into another thread that has not been created yet");
        BOOST_ASSERT_MSG(contains(child, segment_of),
                            "joining a thread that has not been created yet");
        Segment parent_segment = segment_of[parent];
        Segment child_segment = segment_of[child];
        Segment new_parent_segment = ++current_segment;
        segment_of[parent] = new_parent_segment;
        segment_of.erase(child);
        segment_lock.unlock();

        dispatcher.dispatch(JoinEvent(parent_segment, new_parent_segment,
                                                      child_segment));
        dispatcher.dispatch(SegmentHopEvent(parent, new_parent_segment));
        // We could possibly generate informative events like end-of-thread
        // in the child thread, but that's not strictly necessary right now.
    }
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
