/**
 * This file defines the `AcquireEvent` class.
 */

#ifndef D2_ACQUIRE_EVENT_HPP
#define D2_ACQUIRE_EVENT_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/event_traits.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/operators.hpp>


namespace d2 {

/**
 * Represents the acquisition of a resource guarded by a synchronization
 * object in a given thread.
 */
struct AcquireEvent : boost::equality_comparable<AcquireEvent> {
    Thread thread;
    SyncObject lock;
    detail::LockDebugInfo info;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline AcquireEvent() { }

    inline AcquireEvent(SyncObject const& l, Thread const& t)
        : thread(t), lock(l)
    { }

    /**
     * Return whether two `AcquireEvent`s represent the same synchronization
     * object acquired by the same thread.
     */
    friend bool operator==(AcquireEvent const& a, AcquireEvent const& b) {
        return a.lock == b.lock && a.thread == b.thread;
    }

    friend Thread thread_of(AcquireEvent const& self) {
        return self.thread;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, AcquireEvent const& self) {
        os << self.thread << '+' << self.lock << '+' << self.info;
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, AcquireEvent& self) {
        char plus;
        is >> self.thread >> plus >> self.lock >> plus >> self.info;
        return is;
    }

    typedef thread_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_ACQUIRE_EVENT_HPP
