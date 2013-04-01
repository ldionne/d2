/*!
 * @file
 * This file contains unit tests for the wrappers for the `TimedLockable`
 * concept.
 */

#include <d2/timed_lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace {
// non recursive
BOOST_CONCEPT_ASSERT((
    boost::TimedLockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<> >
    >
));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<> >
    >::value
));


// recursive
BOOST_CONCEPT_ASSERT((
    boost::TimedLockable<
        d2::recursive_timed_lockable<d2::detail::timed_lockable_archetype<> >
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_recursive_lockable<
        d2::recursive_timed_lockable<d2::detail::timed_lockable_archetype<> >
    >::value
));


class timed_lockable : public d2::timed_lockable_mixin<timed_lockable> {
    friend class d2::timed_lockable_mixin<timed_lockable>;
    void lock_impl() { }
    void unlock_impl() { }
    bool try_lock_impl() { return false; }
    template <typename Duration>
    bool try_lock_for_impl(Duration const&) { return false; }
    template <typename TimePoint>
    bool try_lock_until_impl(TimePoint const&) { return false; }
};
BOOST_CONCEPT_ASSERT((boost::TimedLockable<timed_lockable>));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<timed_lockable>::value
));


class recursive_timed_lockable
    : public d2::recursive_timed_lockable_mixin<recursive_timed_lockable>
{
    friend class d2::recursive_timed_lockable_mixin<recursive_timed_lockable>;
    void lock_impl() { }
    void unlock_impl() { }
    bool try_lock_impl() { return false; }
    template <typename Duration>
    bool try_lock_for_impl(Duration const&) { return false; }
    template <typename TimePoint>
    bool try_lock_until_impl(TimePoint const&) { return false; }
};
BOOST_CONCEPT_ASSERT((boost::TimedLockable<recursive_timed_lockable>));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<recursive_timed_lockable>::value
));
} // end anonymous namespace


int main() {

}
