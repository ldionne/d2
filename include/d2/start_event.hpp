/**
 * This file defines the `StartEvent` class.
 */

#ifndef D2_START_EVENT_HPP
#define D2_START_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/event_traits.hpp>
#include <d2/thread.hpp>

#include <boost/operators.hpp>
#include <iosfwd>


namespace d2 {

/**
 * Represents the start of a child thread from a parent thread.
 */
struct StartEvent : boost::equality_comparable<StartEvent> {
    Thread parent;
    Thread child;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline StartEvent() { }

    inline StartEvent(Thread const& p, Thread const& c)
        : parent(p), child(c)
    { }

    /**
     * Return whether two `StartEvent`s represent the same parent thread
     * starting the same child thread.
     */
    friend bool operator==(StartEvent const& a, StartEvent const& b) {
        return a.parent == b.parent && a.child == b.child;
    }

    D2_API friend std::ostream& operator<<(std::ostream&, StartEvent const&);
    D2_API friend std::istream& operator>>(std::istream&, StartEvent&);

    typedef process_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_START_EVENT_HPP
