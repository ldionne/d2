/**
 * This file defines the synchronization related events used in the library.
 */

#ifndef D2_CORE_EVENTS_HPP
#define D2_CORE_EVENTS_HPP

#include <d2/core/lock_id.hpp>
#include <d2/core/segment.hpp>
#include <d2/core/thread_id.hpp>
#include <d2/detail/inherit_constructors.hpp>
#include <d2/detail/lock_debug_info.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant.hpp>
#include <dyno/detail/auto_struct.hpp>


namespace d2 {
namespace core {
namespace events {
namespace tag {
    struct thread { };
    struct lock { };
    struct segment { };
    struct parent { };
    struct new_parent { };
    struct child { };
}

#define D2_I_DEFINE_ACCESSORS(z, _, TAG)                                    \
    template <typename Event>                                               \
    typename dyno::detail::get_member_type<Event const, tag::TAG>::type     \
    BOOST_PP_CAT(TAG, _of)(Event const& event) {                            \
        return dyno::detail::get_member<tag::TAG>(event);                   \
    }                                                                       \
                                                                            \
    template <typename Event>                                               \
    typename dyno::detail::get_member_type<Event, tag::TAG>::type           \
    BOOST_PP_CAT(TAG, _of)(Event& event) {                                  \
        return dyno::detail::get_member<tag::TAG>(event);                   \
    }                                                                       \
/**/
BOOST_PP_SEQ_FOR_EACH(
    D2_I_DEFINE_ACCESSORS,~,(thread)(lock)(segment)(parent)(new_parent)(child)
)
#undef D2_I_DEFINE_ACCESSORS


typedef ThreadId thread_id;
typedef LockId lock_id;
typedef Segment segment_id;

struct acquire
    : dyno::detail::auto_struct<
        dyno::detail::member<tag::thread, thread_id>,
        dyno::detail::member<tag::lock, lock_id>
    >
{
    // Default constructor required for variant.
    acquire() { }

    acquire(thread_id tid, lock_id lid)
        : auto_struct_(dyno::detail::make_member<tag::thread>(tid),
                       dyno::detail::make_member<tag::lock>(lid))
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
        ar & boost::serialization::base_object<auto_struct_>(*this)
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
    : dyno::detail::auto_struct<
        dyno::detail::member<tag::parent, segment_id>,
        dyno::detail::member<tag::new_parent, segment_id>,
        dyno::detail::member<tag::child, segment_id>
    >
{
    // Default constructor required for variant.
    start() { }

    start(segment_id parent, segment_id new_parent, segment_id child)
        : auto_struct_(dyno::detail::make_member<tag::parent>(parent),
                       dyno::detail::make_member<tag::new_parent>(new_parent),
                       dyno::detail::make_member<tag::child>(child))
    { }
};

struct join : start {
    join() { }

    D2_INHERIT_CONSTRUCTORS(join, start)
};

struct segment_hop
    : dyno::detail::auto_struct<
        dyno::detail::member<tag::thread, thread_id>,
        dyno::detail::member<tag::segment, segment_id>
    >
{
    segment_hop() { }

    segment_hop(thread_id tid, segment_id sid)
        : auto_struct_(dyno::detail::make_member<tag::thread>(tid),
                       dyno::detail::make_member<tag::segment>(sid))
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
} // end namespace events
} // end namespace core
} // end namespace d2

#endif // !D2_CORE_EVENTS_HPP
