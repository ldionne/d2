/**
 * This file defines the `StartEvent` event.
 */

#ifndef D2_EVENTS_START_EVENT_HPP
#define D2_EVENTS_START_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/events/hold_custom.hpp>
#include <d2/segment.hpp>

#include <dyno/event_scope.hpp>
#include <iosfwd>


namespace d2 {

namespace start_event_detail {
    struct parent { };
    struct new_parent { };
    struct child { };
    D2_DEFINE_CUSTOM_HOLDER(hold_parent, parent, Segment, parent_of)
    D2_DEFINE_CUSTOM_HOLDER(hold_new_parent, new_parent, Segment,new_parent_of)
    D2_DEFINE_CUSTOM_HOLDER(hold_child, child, Segment, child_of)
}

/**
 * Represents the start of a child thread from a parent thread.
 */
struct StartEvent
    : start_event_detail::hold_parent<
        start_event_detail::hold_new_parent<
            start_event_detail::hold_child<>
        >
    >
{
    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    StartEvent() { }

    StartEvent(Segment const& parent, Segment const& new_parent,
                                      Segment const& child) {
        parent_of(*this) = parent;
        new_parent_of(*this) = new_parent;
        child_of(*this) = child;
    }

    D2_API friend std::istream& operator>>(std::istream&, StartEvent&);
    D2_API friend std::ostream& operator<<(std::ostream&, StartEvent const&);

    typedef dyno::process_scope event_scope;
};

} // end namespace d2

#endif // !D2_EVENTS_START_EVENT_HPP
