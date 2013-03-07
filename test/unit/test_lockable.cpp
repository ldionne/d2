/**
 * This file contains unit tests for the `lockable` class.
 */

#include <d2/lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/thread/lockable_concepts.hpp>


namespace {
BOOST_CONCEPT_ASSERT((
    boost::Lockable<
        d2::lockable<d2::detail::lockable_archetype<> >
    >
));

BOOST_MPL_ASSERT((
    boost::sync::is_lockable<
        d2::lockable<d2::detail::lockable_archetype<> >
    >
));
} // end anonymous namespace


int main() {

}
