/**
 * This file defines the different event types captured by the logging system.
 */

#ifndef D2_EVENTS_HPP
#define D2_EVENTS_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/types.hpp>

#include <boost/operators.hpp>
#include <boost/variant.hpp>


namespace d2 {

/**
 * Represents the acquisition of a resource guarded by a synchronization
 * object in a given thread.
 */
struct AcquireEvent : boost::equality_comparable<AcquireEvent> {
    SyncObject lock;
    Thread thread;
    detail::lock_debug_info info;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline AcquireEvent() { }

    inline AcquireEvent(SyncObject const& l, Thread const& t)
        : lock(l), thread(t)
    { }

    /**
     * Return whether two `AcquireEvent`s represent the same synchronization
     * object acquired by the same thread.
     */
    friend bool operator==(AcquireEvent const& a, AcquireEvent const& b) {
        return a.lock == b.lock && a.thread == b.thread;
    }
};

/**
 * Represents the release of a resource guarded by a synchronization
 * object in a given thread.
 */
struct ReleaseEvent : boost::equality_comparable<ReleaseEvent> {
    SyncObject lock;
    Thread thread;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline ReleaseEvent() { }

    inline ReleaseEvent(SyncObject const& l, Thread const& t)
        : lock(l), thread(t)
    { }

    /**
     * Return whether two `ReleaseEvent`s represent the same synchronization
     * object released by the same thread.
     */
    friend bool operator==(ReleaseEvent const& a, ReleaseEvent const& b) {
        return a.lock == b.lock && a.thread == b.thread;
    }
};

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
};

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
};

/**
 * Represents any type of event.
 */
typedef boost::variant<AcquireEvent, ReleaseEvent,
                       StartEvent, JoinEvent> Event;

} // end namespace d2

#endif // !D2_EVENTS_HPP
