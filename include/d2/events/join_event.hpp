/**
 * This file defines the `JoinEvent` event.
 */

#ifndef D2_EVENTS_JOIN_EVENT_HPP
#define D2_EVENTS_JOIN_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/events/start_event.hpp>
#include <d2/segment.hpp>

#include <iosfwd>


namespace d2 {

/**
 * Represents the joining of a child thread into its parent thread.
 */
struct JoinEvent : StartEvent {
    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    JoinEvent() { }

    JoinEvent(Segment const& parent, Segment const& new_parent,
                                     Segment const& child)
        : StartEvent(parent, new_parent, child)
    { }

    D2_API friend std::istream& operator>>(std::istream&, JoinEvent&);
    D2_API friend std::ostream& operator<<(std::ostream&, JoinEvent const&);
};

} // end namespace d2

#endif // !D2_EVENTS_JOIN_EVENT_HPP
