/**
 * This file defines the `AcquireEvent` event.
 */

#ifndef D2_EVENTS_ACQUIRE_EVENT_HPP
#define D2_EVENTS_ACQUIRE_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/event_traits.hpp>
#include <d2/events/hold_custom.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <iosfwd>


namespace d2 {

namespace acquire_event_detail {
    struct lock { };
    struct thread { };
    D2_DEFINE_CUSTOM_HOLDER(hold_lock, lock, LockId, lock_of)
    D2_DEFINE_CUSTOM_HOLDER(hold_thread, thread, ThreadId, thread_of)
}

/**
 * Represents the acquisition of a resource guarded by a synchronization
 * object in a given thread.
 */
struct AcquireEvent
    : acquire_event_detail::hold_lock<
        acquire_event_detail::hold_thread<>
    >
{
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

    D2_API friend std::istream& operator>>(std::istream&, AcquireEvent&);
    D2_API friend std::ostream& operator<<(std::ostream&, AcquireEvent const&);

    typedef thread_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_EVENTS_ACQUIRE_EVENT_HPP
