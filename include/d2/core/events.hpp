/**
 * This file defines the synchronization related events used in the library.
 */

#ifndef D2_CORE_EVENTS_HPP
#define D2_CORE_EVENTS_HPP

#include <d2/detail/inherit_constructors.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/lock_id.hpp>
#include <d2/segment.hpp>
#include <d2/thread_id.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant.hpp>
#include <dyno/auto_event.hpp>


namespace d2 {
namespace core {
namespace events {
namespace tag {
    struct thread { };
    struct lock { };
    struct segment { };
    struct parent_segment { };
    struct new_parent_segment { };
    struct child_segment { };
}

typedef ThreadId thread_id;
typedef LockId lock_id;
typedef Segment segment_id;

struct acquire
    : dyno::auto_event<
        dyno::members<
            tag::thread, thread_id,
            tag::lock, lock_id
        >
    >
{
    // Default constructor required for variant.
    acquire() { }

    acquire(thread_id tid, lock_id lid)
        : auto_event_(dyno::member<tag::thread>(tid),
                      dyno::member<tag::lock>(lid))
    { }

    // This is temporary, for compatibility with the old events.
    typedef detail::LockDebugInfo aux_info_type;
    aux_info_type info;

    friend aux_info_type const& aux_info_of(acquire const& self) {
        return self.info;
    }

    friend aux_info_type& aux_info_of(acquire& self) {
        return self.info;
    }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & boost::serialization::base_object<auto_event_>(*this)
           & info;
    }
};

struct release : acquire {
    release() { }

    D2_INHERIT_CONSTRUCTORS(release, acquire)
};

struct recursive_acquire : acquire {
    recursive_acquire() { }

    D2_INHERIT_CONSTRUCTORS(recursive_acquire, acquire)
};

struct recursive_release : release {
    recursive_release() { }

    D2_INHERIT_CONSTRUCTORS(recursive_release, release)
};

struct start
    : dyno::auto_event<
        dyno::members<
            tag::parent_segment, segment_id,
            tag::new_parent_segment, segment_id,
            tag::child_segment, segment_id
        >
    >
{
    // Default constructor required for variant.
    start() { }

    start(segment_id parent_segment, segment_id new_parent_segment, segment_id child_segment)
        : auto_event_(dyno::member<tag::parent_segment>(parent_segment),
                      dyno::member<tag::new_parent_segment>(new_parent_segment),
                      dyno::member<tag::child_segment>(child_segment))
    { }
};

struct join : start {
    join() { }

    D2_INHERIT_CONSTRUCTORS(join, start)
};

struct segment_hop
    : dyno::auto_event<
        dyno::members<
            tag::thread, thread_id,
            tag::segment, segment_id
        >
    >
{
    segment_hop() { }

    segment_hop(thread_id tid, segment_id sid)
        : auto_event_(dyno::member<tag::thread>(tid),
                      dyno::member<tag::segment>(sid))
    { }
};

/**
 * Variant holding events not specific to a single thread.
 */
typedef boost::variant<start, join> non_thread_specific;

/**
 * Variant holding events specific to a single thread.
 */
typedef boost::variant<
            acquire, release, recursive_acquire, recursive_release, segment_hop
        > thread_specific;

/**
 * Metafunction returning whether an event is a thread specific event.
 */
template <typename Event>
struct is_thread_specific
    : boost::mpl::contains<
        boost::mpl::vector<
            acquire, release, recursive_acquire, recursive_release, segment_hop
        >,
        Event
    >
{ };

template <> struct is_thread_specific<start> : boost::mpl::false_ { };
template <> struct is_thread_specific<join> : boost::mpl::false_ { };

template <typename Event>
typename Event::template get_const_type<tag::thread>::type
thread_of(Event const& event) {
    return get(tag::thread(), event);
}

template <typename Event>
typename Event::template get_type<tag::thread>::type thread_of(Event& event) {
    return get(tag::thread(), event);
}


template <typename Event>
typename Event::template get_const_type<tag::lock>::type
lock_of(Event const& event) {
    return get(tag::lock(), event);
}

template <typename Event>
typename Event::template get_type<tag::lock>::type lock_of(Event& event) {
    return get(tag::lock(), event);
}


template <typename Event>
typename Event::template get_const_type<tag::segment>::type
segment_of(Event const& event) {
    return get(tag::segment(), event);
}

template <typename Event>
typename Event::template get_type<tag::segment>::type
segment_of(Event& event) {
    return get(tag::segment(), event);
}


template <typename Event>
typename Event::template get_const_type<tag::parent_segment>::type
parent_of(Event const& event) {
    return get(tag::parent_segment(), event);
}

template <typename Event>
typename Event::template get_type<tag::parent_segment>::type
parent_of(Event& event) {
    return get(tag::parent_segment(), event);
}


template <typename Event>
typename Event::template get_const_type<tag::new_parent_segment>::type
new_parent_of(Event const& event) {
    return get(tag::new_parent_segment(), event);
}

template <typename Event>
typename Event::template get_type<tag::new_parent_segment>::type
new_parent_of(Event& event) {
    return get(tag::new_parent_segment(), event);
}


template <typename Event>
typename Event::template get_const_type<tag::child_segment>::type
child_of(Event const& event) {
    return get(tag::child_segment(), event);
}

template <typename Event>
typename Event::template get_type<tag::child_segment>::type
child_of(Event& event) {
    return get(tag::child_segment(), event);
}
} // end namespace events
} // end namespace core
} // end namespace d2

#endif // !D2_CORE_EVENTS_HPP
