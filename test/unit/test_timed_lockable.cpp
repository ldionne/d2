/**
 * This file contains unit tests for the `timed_lockable` class.
 */

#include <d2/timed_lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace {
BOOST_CONCEPT_ASSERT((
    boost::TimedLockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<>, false>
    >
));

BOOST_CONCEPT_ASSERT((
    boost::TimedLockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<>, true>
    >
));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<>, false>
    >::value
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_recursive_lockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<>, true>
    >::value
));
} // end anonymous namespace


int main() {

}
