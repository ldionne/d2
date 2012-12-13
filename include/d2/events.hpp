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

namespace detail {
    typedef boost::variant<acquire_event, release_event,
                           start_event, join_event> event_types;
}

/**
 * Represents any type of event.
 * @note We can't use a typedef because we can't define our own output stream
 *       operator then.
 */
struct event : detail::event_types {
    inline event() { }
    template <typename T> event(T const& t) : detail::event_types(t) { }

    template <typename T>
    event& operator=(T const& t) {
        detail::event_types::operator=(t);
        return *this;
    }
};

namespace detail {
    namespace mpl = boost::mpl;
    typedef mpl::map<
        mpl::pair<acquire_event, mpl::char_<'a'> >,
        mpl::pair<release_event, mpl::char_<'r'> >,
        mpl::pair<start_event, mpl::char_<'s'> >,
        mpl::pair<join_event, mpl::char_<'j'> >
    > output_event_tags;

    template <typename Ostream>
    struct output_event_visitor : boost::static_visitor<> {
        Ostream& os_;
        explicit output_event_visitor(Ostream& os) : os_(os) { }

        template <typename Event>
        void operator()(Event const& e) {
            os_ << mpl::at<output_event_tags, Event>::type::value << e;
        }
    };

    template <typename First, typename Last>
    struct try_input {
        template <typename Istream>
        static void call(Istream& is, event& e, char tag) {
            typedef typename mpl::deref<First>::type EventTag;
            if (EventTag::second::value == tag) {
                typename EventTag::first tmp;
                is >> tmp;
                e = tmp;
            }
            else {
                typedef typename mpl::next<First>::type Next;
                try_input<Next, Last>::call(is, e, tag);
            }
        }
    };

    template <typename Last>
    struct try_input<Last, Last> {
        template <typename Istream>
        static void call(Istream&, event&, char) { }
    };
} // end namespace detail

template <typename Ostream>
Ostream& operator<<(Ostream& os, event const& e) {
    detail::output_event_visitor<Ostream> visitor(os);
    boost::apply_visitor(visitor, e);
    return os;
}

template <typename Istream>
Istream& operator>>(Istream& is, event& e) {
    char tag;
    is >> tag;
    detail::try_input<
        typename boost::mpl::begin<detail::output_event_tags>::type,
        typename boost::mpl::end<detail::output_event_tags>::type
    >::call(is, e, tag);
    return is;
}

} // end namespace d2

#endif // !D2_EVENTS_HPP
