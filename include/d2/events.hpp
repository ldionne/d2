/**
 * This file defines the different event types captured by the logging system.
 */

#ifndef D2_EVENTS_HPP
#define D2_EVENTS_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/types.hpp>

#include <boost/assert.hpp>
#include <boost/mpl/char.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/operators.hpp>
#include <boost/variant.hpp>
#include <ios>


namespace d2 {

/**
 * Represents the acquisition of a resource guarded by a synchronization
 * object in a given thread.
 */
struct acquire_event : boost::equality_comparable<acquire_event> {
    sync_object lock;
    class thread thread;
    detail::lock_debug_info info;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline acquire_event() { }

    inline acquire_event(sync_object const& l, class thread const& t)
        : lock(l), thread(t)
    { }

    /**
     * Save an `acquire_event` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, acquire_event const& e) {
        return os << e.lock << ' ' << e.thread << ' ' << e.info, os;
    }

    /**
     * Return whether two `acquire_event`s represent the same synchronization
     * object acquired by the same thread.
     */
    friend bool operator==(acquire_event const& a, acquire_event const& b) {
        return a.lock == b.lock && a.thread == b.thread;
    }

    /**
     * Load an `acquire_event` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, acquire_event& e) {
        BOOST_ASSERT_MSG(is.flags() & std::ios::skipws,
   "the input stream must have the skipws flag set to load an acquire_event");
        return is >> e.lock >> e.thread >> e.info, is;
    }
};

/**
 * Represents the release of a resource guarded by a synchronization
 * object in a given thread.
 */
struct release_event : boost::equality_comparable<release_event> {
    sync_object lock;
    class thread thread;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline release_event() { }

    inline release_event(sync_object const& l, class thread const& t)
        : lock(l), thread(t)
    { }

    /**
     * Save a `release_event` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, release_event const& e) {
        return os << e.lock << ' ' << e.thread, os;
    }

    /**
     * Return whether two `release_event`s represent the same synchronization
     * object released by the same thread.
     */
    friend bool operator==(release_event const& a, release_event const& b) {
        return a.lock == b.lock && a.thread == b.thread;
    }

    /**
     * Load a `release_event` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, release_event& e) {
        BOOST_ASSERT_MSG(is.flags() & std::ios::skipws,
    "the input stream must have the skipws flag set to load a release_event");
        return is >> e.lock >> e.thread, is;
    }
};

/**
 * Represents the start of a child thread from a parent thread.
 */
struct start_event : boost::equality_comparable<start_event> {
    thread parent;
    thread child;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline start_event() { }

    inline start_event(thread const& p, thread const& c)
        : parent(p), child(c)
    { }

    /**
     * Return whether two `start_event`s represent the same parent thread
     * starting the same child thread.
     */
    friend bool operator==(start_event const& a, start_event const& b) {
        return a.parent == b.parent && a.child == b.child;
    }

    /**
     * Save a `start_event` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, start_event const& e) {
        return os << e.parent << ' ' << e.child, os;
    }

    /**
     * Load a `start_event` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, start_event& e) {
        BOOST_ASSERT_MSG(is.flags() & std::ios::skipws,
    "the input stream must have the skipws flag set to load a start_event");
        return is >> e.parent >> e.child, is;
    }
};

/**
 * Represents the joining of a child thread into its parent thread.
 */
struct join_event {
    thread parent;
    thread child;

    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    inline join_event() { }

    inline join_event(thread const& p, thread const& c)
        : parent(p), child(c)
    { }

    /**
     * Return whether two `join_event`s represent the same parent thread
     * joining the same child thread.
     */
    friend bool operator==(join_event const& a, join_event const& b) {
        return a.parent == b.parent && a.child == b.child;
    }

    /**
     * Save a `join_event` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, join_event const& e) {
        return os << e.parent << ' ' << e.child, os;
    }

    /**
     * Load a `join_event` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, join_event& e) {
        BOOST_ASSERT_MSG(is.flags() & std::ios::skipws,
    "the input stream must have the skipws flag set to load a join_event");
        return is >> e.parent >> e.child, is;
    }
};

/**
 * Represents any type of event.
 */
typedef boost::variant<acquire_event, release_event,
                       start_event, join_event> event;

} // end namespace d2

#endif // !D2_EVENTS_HPP
