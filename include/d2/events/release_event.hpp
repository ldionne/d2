/**
 * This file defines the `ReleaseEvent` class.
 */

#ifndef D2_EVENTS_RELEASE_EVENT_HPP
#define D2_EVENTS_RELEASE_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/events/acquire_event.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <iosfwd>


namespace d2 {

/**
 * Represents the release of a resource guarded by a synchronization
 * object in a given thread.
 */
struct ReleaseEvent : AcquireEvent {
    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    ReleaseEvent() { }

    ReleaseEvent(LockId const& l, ThreadId const& t)
        : AcquireEvent(l, t)
    { }

    D2_API friend std::ostream& operator<<(std::ostream&, ReleaseEvent const&);
    D2_API friend std::istream& operator>>(std::istream&, ReleaseEvent&);
};

} // end namespace d2

#endif // !D2_EVENTS_RELEASE_EVENT_HPP
