/**
 * This file defines the `ReleaseEvent` class.
 */

#ifndef D2_RELEASE_EVENT_HPP
#define D2_RELEASE_EVENT_HPP

#include <d2/event_traits.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/operators.hpp>
#include <iosfwd>


namespace d2 {

/**
 * Represents the release of a resource guarded by a synchronization
 * object in a given thread.
 */
struct ReleaseEvent : boost::equality_comparable<ReleaseEvent> {
    Thread thread;
    SyncObject lock;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline ReleaseEvent() { }

    inline ReleaseEvent(SyncObject const& l, Thread const& t)
        : thread(t), lock(l)
    { }

    /**
     * Return whether two `ReleaseEvent`s represent the same synchronization
     * object released by the same thread.
     */
    friend bool operator==(ReleaseEvent const& a, ReleaseEvent const& b) {
        return a.lock == b.lock && a.thread == b.thread;
    }

    friend Thread thread_of(ReleaseEvent const& self) {
        return self.thread;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, ReleaseEvent const& self) {
        os << self.thread << '-' << self.lock << '-';
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, ReleaseEvent& self) {
        char minus;
        is >> self.thread >> minus >> self.lock >> minus;
        return is;
    }

    typedef thread_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_RELEASE_EVENT_HPP
