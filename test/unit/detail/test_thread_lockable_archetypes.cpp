/**
 * This file contains unit tests for the lockable concept archetypes.
 */

#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/thread/lockable_concepts.hpp>


namespace {
BOOST_CONCEPT_ASSERT((
    boost::BasicLockable<d2::detail::basic_lockable_archetype<> >
));

BOOST_CONCEPT_ASSERT((
    boost::Lockable<d2::detail::lockable_archetype<> >
));

BOOST_CONCEPT_ASSERT((
    boost::TimedLockable<d2::detail::timed_lockable_archetype<> >
));
} // end anonymous namespace


int main() {

}
