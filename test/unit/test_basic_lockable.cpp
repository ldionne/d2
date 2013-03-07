/**
 * This file contains unit tests for the `basic_lockable` class.
 */

#include <d2/basic_lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/thread/lockable_concepts.hpp>


namespace {
BOOST_CONCEPT_ASSERT((
    boost::BasicLockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<> >
    >
));

BOOST_MPL_ASSERT((
    boost::sync::is_basic_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<> >
    >
));
} // end anonymous namespace


int main() {

}
