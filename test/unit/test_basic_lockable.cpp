/**
 * This file contains unit tests for the `basic_lockable` class.
 */

#include <d2/basic_lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace {
BOOST_CONCEPT_ASSERT((
    boost::BasicLockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<>, false>
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_basic_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<>, false>
    >::value
));

BOOST_CONCEPT_ASSERT((
    boost::BasicLockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<>, true>
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_basic_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<>, true>
    >::value
));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<>, false>
    >::value
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_recursive_basic_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<>, true>
    >::value
));
} // end anonymous namespace


int main() {

}
