/**
 * This file defines archetypes for the lockable concepts of `Boost.Thread`.
 */

#ifndef D2_DETAIL_THREAD_LOCKABLE_ARCHETYPES_HPP
#define D2_DETAIL_THREAD_LOCKABLE_ARCHETYPES_HPP

#include <boost/chrono/system_clocks.hpp>
#include <boost/concept_archetype.hpp>


namespace d2 {
namespace thread_lockable_archetypes_detail {
//! Archetype for the `BasicLockable` concept.
template <typename Base = boost::null_archetype<> >
struct basic_lockable_archetype : Base {
    void lock() { }
    void unlock() { }
};

//! Archetype for the `Lockable` concept.
template <typename Base = boost::null_archetype<> >
struct lockable_archetype : Base {
    void lock() { }
    void unlock() { }
    bool try_lock() { return false; }
};

//! Archetype for the `TimedLockable` concept.
template <typename Base = boost::null_archetype<> >
struct timed_lockable_archetype : Base {
    void lock() { }
    void unlock() { }
    bool try_lock() { return false; }

    bool try_lock_until(boost::chrono::system_clock::time_point const&) {
        return false;
    }

    bool try_lock_for(boost::chrono::system_clock::duration const&) {
        return false;
    }
};
} // end namespace thread_lockable_archetypes_detail

namespace detail {
    using thread_lockable_archetypes_detail::basic_lockable_archetype;
    using thread_lockable_archetypes_detail::lockable_archetype;
    using thread_lockable_archetypes_detail::timed_lockable_archetype;
}
} // end namespace d2

#endif // !D2_DETAIL_THREAD_LOCKABLE_ARCHETYPES_HPP
