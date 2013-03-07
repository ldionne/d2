/**
 * This file contains unit tests for the `timed_lockable` class.
 */

#include <d2/timed_lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/thread/lockable_concepts.hpp>


namespace {
BOOST_CONCEPT_ASSERT((
    boost::TimedLockable<
        d2::timed_lockable<d2::detail::timed_lockable_archetype<> >
    >
));
} // end anonymous namespace


int main() {

}
