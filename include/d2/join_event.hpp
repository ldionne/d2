/**
 * This file defines the `JoinEvent` class.
 */

#ifndef D2_JOIN_EVENT_HPP
#define D2_JOIN_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/event_traits.hpp>
#include <d2/thread.hpp>

#include <boost/operators.hpp>
#include <iosfwd>


namespace d2 {

/**
 * Represents the joining of a child thread into its parent thread.
 */
struct JoinEvent {
    Thread parent;
    Thread child;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline JoinEvent() { }

    inline JoinEvent(Thread const& p, Thread const& c)
        : parent(p), child(c)
    { }

    /**
     * Return whether two `JoinEvent`s represent the same parent thread
     * joining the same child thread.
     */
    friend bool operator==(JoinEvent const& a, JoinEvent const& b) {
        return a.parent == b.parent && a.child == b.child;
    }

    D2_API friend std::ostream& operator<<(std::ostream&, JoinEvent const&);
    D2_API friend std::istream& operator>>(std::istream&, JoinEvent&);

    typedef process_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_JOIN_EVENT_HPP
