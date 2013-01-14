/**
 * This file defines the `StartEvent` event.
 */

#ifndef D2_EVENTS_START_EVENT_HPP
#define D2_EVENTS_START_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/event_traits.hpp>
#include <d2/segment.hpp>

#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <iosfwd>


namespace d2 {

/**
 * Represents the start of a child thread from a parent thread.
 */
struct StartEvent : boost::equality_comparable<StartEvent> {
    Segment parent;
    Segment new_parent;
    Segment child;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline StartEvent() { }

    inline StartEvent(Segment const& parent,
                      Segment const& new_parent,
                      Segment const& child)
        : parent(parent), new_parent(new_parent), child(child)
    { }

    /**
     * Return whether two `StartEvent`s represent the same parent thread
     * starting the same child thread.
     */
    friend bool operator==(StartEvent const& a, StartEvent const& b) {
        return a.parent == b.parent &&
               a.new_parent == b.new_parent &&
               a.child == b.child;
    }

    D2_API friend std::istream& operator>>(std::istream&, StartEvent&);
    D2_API friend std::ostream& operator<<(std::ostream&, StartEvent const&);

    typedef process_scope event_scope;
    typedef strict_order_policy ordering_policy;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & parent & new_parent & child;
    }
};

} // end namespace d2

#endif // !D2_EVENTS_START_EVENT_HPP
