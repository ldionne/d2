/**
 * This file defines the `AcquireEvent` event.
 */

#ifndef D2_EVENTS_ACQUIRE_EVENT_HPP
#define D2_EVENTS_ACQUIRE_EVENT_HPP

#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>
#include <d2/detail/config.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/event.hpp>
#include <d2/event_traits.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <iosfwd>


namespace d2 {

namespace acquire_event_detail {
    struct thread { };
    struct lock { };
    typedef boost::fusion::vector<
                boost::fusion::pair<thread, ThreadId>,
                boost::fusion::pair<lock, LockId>
            > members;
} // end namespace acquire_event_detail

/**
 * Represents the acquisition of a resource guarded by a synchronization
 * object in a given thread.
 */
struct AcquireEvent : Event<AcquireEvent, acquire_event_detail::members> {
    detail::LockDebugInfo info;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    AcquireEvent() { }

    AcquireEvent(LockId const& l, ThreadId const& t) {
        lock_of(*this) = l;
        thread_of(*this) = t;
    }

    friend ThreadId const& thread_of(AcquireEvent const& self) {
        return get(acquire_event_detail::thread(), self);
    }

    friend ThreadId& thread_of(AcquireEvent& self) {
        return get(acquire_event_detail::thread(), self);
    }

    friend LockId const& lock_of(AcquireEvent const& self) {
        return get(acquire_event_detail::lock(), self);
    }

    friend LockId& lock_of(AcquireEvent& self) {
        return get(acquire_event_detail::lock(), self);
    }

    D2_API friend std::istream& operator>>(std::istream&, AcquireEvent&);
    D2_API friend std::ostream& operator<<(std::ostream&, AcquireEvent const&);

    typedef thread_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_EVENTS_ACQUIRE_EVENT_HPP
