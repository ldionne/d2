/**
 * This file contains compile-time tests for the concepts used within `d2`.
 */

#include <d2/concepts.hpp>

#include <boost/concept/assert.hpp>
#include <cstddef>


namespace {
BOOST_CONCEPT_ASSERT((
    d2::UniquelyIdentifiable<
        d2::uniquely_identifiable_archetype<>
    >
));

BOOST_CONCEPT_ASSERT((
    d2::UniquelyIdentifiable<std::size_t>
));
} // end anonymous namespace

int main() {

}
