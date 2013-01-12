/**
 * This file defines the `JoinEvent` event.
 */

#ifndef D2_EVENTS_JOIN_EVENT_HPP
#define D2_EVENTS_JOIN_EVENT_HPP

#include <d2/event_traits.hpp>
#include <d2/segment.hpp>

#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iosfwd>


namespace d2 {

/**
 * Represents the joining of a child thread into its parent thread.
 */
struct JoinEvent : boost::equality_comparable<JoinEvent> {
    Segment parent;
    Segment new_parent;
    Segment child;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline JoinEvent() { }

    inline JoinEvent(Segment const& parent,
                     Segment const& new_parent,
                     Segment const& child)
        : parent(parent), new_parent(new_parent), child(child)
    { }

    /**
     * Return whether two `JoinEvent`s represent the same parent thread
     * joining the same child thread.
     */
    friend bool operator==(JoinEvent const& a, JoinEvent const& b) {
        return a.parent == b.parent &&
               a.new_parent == b.new_parent &&
               a.child == b.child;
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os, JoinEvent const& self) {
        os << self.parent << '^' << self.new_parent << '^' << self.child
                                                                    << '^';
        return os;
    }

    template <typename CharT, typename Traits>
    friend std::basic_istream<CharT, Traits>&
    operator>>(std::basic_istream<CharT, Traits>& is, JoinEvent& self) {
        using namespace boost::spirit::qi;

        unsigned long parent, new_parent, child;
        is >> match(ulong_ >> '^' >> ulong_ >> '^' >> ulong_ >> '^',
                    parent, new_parent, child);
        self = JoinEvent();
        self.parent += parent;
        self.new_parent += new_parent;
        self.child += child;
        return is;
    }

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

#endif // !D2_EVENTS_JOIN_EVENT_HPP
