/**
 * This file defines the `AcquireEvent` event.
 */

#ifndef D2_EVENTS_ACQUIRE_EVENT_HPP
#define D2_EVENTS_ACQUIRE_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/event_traits.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <iosfwd>


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

    D2_API friend std::istream& operator>>(std::istream&, AcquireEvent&);
    D2_API friend std::ostream& operator<<(std::ostream&, AcquireEvent const&);

    typedef thread_scope event_scope;
    typedef strict_order_policy ordering_policy;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & thread & lock & info;
    }
};

} // end namespace d2

#endif // !D2_EVENTS_ACQUIRE_EVENT_HPP
