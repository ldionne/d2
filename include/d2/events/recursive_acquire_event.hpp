/**
 * This file defines the `RecursiveAcquireEvent` event.
 */

#ifndef D2_EVENTS_RECURSIVE_ACQUIRE_EVENT_HPP
#define D2_EVENTS_RECURSIVE_ACQUIRE_EVENT_HPP

#include <d2/events/acquire_event.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <iostream>


namespace d2 {

/**
 * Represents the same as an `AcquireEvent`, but the represented
 * synchronization object can be recursively acquired.
 *
 * @see `AcquireEvent`.
 * @internal Even though inheritance is used, this class is in NO WAY meant
 *           to be polymorphic.
 */
struct RecursiveAcquireEvent : AcquireEvent {
    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    RecursiveAcquireEvent() { }

    RecursiveAcquireEvent(LockId const& l, ThreadId const& t)
        : AcquireEvent(l, t)
    { }

    friend std::istream& operator>>(std::istream& is,
                                    RecursiveAcquireEvent& self) {
        char flag = 'X';
        if (is >> flag) {
            if (flag == 'r')
                is >> static_cast<AcquireEvent&>(self);
            else
                is.setstate(std::ios_base::failbit);
        }
        return is;
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    RecursiveAcquireEvent const& self) {
        return os << 'r' << static_cast<AcquireEvent const&>(self);
    }
};

} // end namespace d2

#endif // !D2_EVENTS_RECURSIVE_ACQUIRE_EVENT_HPP
