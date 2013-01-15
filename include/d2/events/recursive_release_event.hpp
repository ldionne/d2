/**
 * This file defines the `RecursiveReleaseEvent` event.
 */

#ifndef D2_EVENTS_RECURSIVE_RELEASE_EVENT_HPP
#define D2_EVENTS_RECURSIVE_RELEASE_EVENT_HPP

#include <d2/events/release_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <iostream>


namespace d2 {

/**
 * Represents the same as a `ReleaseEvent`, but the represented
 * synchronization object can be recursively released.
 *
 * @see `ReleaseEvent`.
 * @internal Even though inheritance is used, this class is in NO WAY meant
 *           to be polymorphic.
 */
struct RecursiveReleaseEvent : ReleaseEvent {
    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    RecursiveReleaseEvent() { }

    RecursiveReleaseEvent(SyncObject const& l, Thread const& t)
        : ReleaseEvent(l, t)
    { }

    friend std::istream& operator>>(std::istream& is,
                                    RecursiveReleaseEvent& self) {
        char flag = 'X';
        if (is >> flag) {
            if (flag == 'r')
                is >> static_cast<ReleaseEvent&>(self);
            else
                is.setstate(std::ios_base::failbit);
        }
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    RecursiveReleaseEvent const& self) {
        return os << 'r' << static_cast<ReleaseEvent const&>(self);
    }
};

} // end namespace d2

#endif // !D2_EVENTS_RECURSIVE_RELEASE_EVENT_HPP
