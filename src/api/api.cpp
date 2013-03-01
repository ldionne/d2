/**
 * This file implements the _d2_ API.
 */

#define D2_SOURCE
#include <d2/api.h>
#include <d2/core/filesystem_dispatcher.hpp>
#include <d2/detail/atomic.hpp>
#include <d2/detail/config.hpp>
#include <d2/detail/mutex.hpp>
#include <d2/events.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <stddef.h>


namespace d2 { namespace api_detail {
    static core::FilesystemDispatcher dispatcher;
    static detail::atomic<bool> event_logging_enabled(false);
}}

D2_API extern void d2_disable_event_logging(void) {
    d2::api_detail::event_logging_enabled = false;
}

D2_API extern void d2_enable_event_logging(void) {
    d2::api_detail::event_logging_enabled = true;
}

D2_API extern int d2_is_enabled(void) {
    return d2::api_detail::event_logging_enabled ? 1 : 0;
}

D2_API extern int d2_is_disabled(void) {
    return d2_is_enabled() ? 0 : 1;
}

D2_API extern int d2_set_log_repository(char const* path) {
    // Note: 0 for success and anything else but 0 for failure.
    return d2::api_detail::dispatcher.set_repository_noexcept(path) ? 0 : 1;
}

D2_API extern void d2_unset_log_repository(void) {
    d2::api_detail::dispatcher.unset_repository();
}

D2_API extern void d2_notify_acquire(size_t thread_id, size_t lock_id) {
    using namespace d2;
    using namespace d2::api_detail;
    if (d2_is_enabled()) {
        AcquireEvent event((LockId(lock_id)), ThreadId(thread_id));
                        // ignore current frame
        event.info.init_call_stack(1);
        dispatcher.dispatch(event);
    }
}

D2_API extern void d2_notify_recursive_acquire(size_t thread_id,
                                               size_t lock_id) {
    using namespace d2;
    using namespace d2::api_detail;
    if (d2_is_enabled()) {
        RecursiveAcquireEvent event((LockId(lock_id)), ThreadId(thread_id));
                        // ignore current frame
        event.info.init_call_stack(1);
        dispatcher.dispatch(event);
    }
}

D2_API extern void d2_notify_release(size_t thread_id, size_t lock_id) {
    using namespace d2;
    using namespace d2::api_detail;
    if (d2_is_enabled())
        dispatcher.dispatch(ReleaseEvent((LockId(lock_id)),
                                          ThreadId(thread_id)));
}

D2_API extern void d2_notify_recursive_release(size_t thread_id,
                                               size_t lock_id) {
    using namespace d2;
    using namespace d2::api_detail;
    if (d2_is_enabled())
        dispatcher.dispatch(RecursiveReleaseEvent((LockId(lock_id)),
                                                  ThreadId(thread_id)));
}

namespace d2 { namespace api_detail {
    static detail::atomic<std::size_t> lock_id_counter(0);
}}

D2_API extern size_t d2_get_lock_id(void) {
    return d2::api_detail::lock_id_counter++;
}

namespace d2 { namespace api_detail {
    // default initialized to the initial segment value
    static Segment current_segment;
    static detail::mutex segment_mutex;
    static boost::unordered_map<ThreadId, Segment> segment_of;

    template <typename Value, typename Container>
    bool contains(Value const& v, Container const& c) {
        return c.find(v) != c.end();
    }
}}

D2_API extern void d2_notify_start(size_t parent_id, size_t child_id) {
    using namespace d2;
    using namespace d2::api_detail;
    if (d2_is_enabled()) {
        ThreadId parent(parent_id), child(child_id);
        Segment parent_segment, child_segment, new_parent_segment;
        {
            detail::scoped_lock<detail::mutex> lock(segment_mutex);
            BOOST_ASSERT_MSG(parent != child, "thread starting itself");
            BOOST_ASSERT_MSG(segment_of.empty() || contains(parent,segment_of),
        "starting a thread from another thread that has not been created yet");
            // segment_of[parent] will be the initial segment value on the
            // very first call, which is the same as current_segment. so this
            // means two things:
            //  - parent_segment will be the initial segment value on the very
            //    first call, and the segment of `parent` on subsequent calls,
            //    which is fine.
            //  - we must PREincrement the current_segment so it is distinct
            //    from the initial value.
            Segment& segment_of_parent = segment_of[parent];
            parent_segment = segment_of_parent;
            new_parent_segment = ++current_segment;
            child_segment = ++current_segment;
            segment_of[child] = child_segment;
            segment_of_parent = new_parent_segment;
        }

        dispatcher.dispatch(StartEvent(parent_segment, new_parent_segment,
                                                       child_segment));
        dispatcher.dispatch(SegmentHopEvent(parent, new_parent_segment));
        dispatcher.dispatch(SegmentHopEvent(child, child_segment));
    }
}

D2_API extern void d2_notify_join(size_t parent_id, size_t child_id) {
    using namespace d2;
    using namespace d2::api_detail;
    if (d2_is_enabled()) {
        ThreadId parent(parent_id), child(child_id);
        Segment parent_segment, child_segment, new_parent_segment;
        {
            detail::scoped_lock<detail::mutex> lock(segment_mutex);
            BOOST_ASSERT_MSG(parent != child, "thread joining itself");
            BOOST_ASSERT_MSG(contains(parent, segment_of),
        "joining a thread into another thread that has not been created yet");
            BOOST_ASSERT_MSG(contains(child, segment_of),
                            "joining a thread that has not been created yet");
            Segment& segment_of_parent = segment_of[parent];
            parent_segment = segment_of_parent;
            child_segment = segment_of[child];
            new_parent_segment = ++current_segment;
            segment_of_parent = new_parent_segment;
            segment_of.erase(child);
        }

        dispatcher.dispatch(JoinEvent(parent_segment, new_parent_segment,
                                                      child_segment));
        dispatcher.dispatch(SegmentHopEvent(parent, new_parent_segment));
        // We could possibly generate informative events like end-of-thread
        // in the child thread, but that's not strictly necessary right now.
    }
}
