/**
 * This file contains unit tests for the `uniquely_identifiable.hpp` header.
 */

#include <d2/uniquely_identifiable.hpp>

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

struct with_unique_id : d2::uniquely_identifiable<with_unique_id> { };
BOOST_CONCEPT_ASSERT((d2::UniquelyIdentifiable<with_unique_id>));
} // end anonymous namespace


int main() {

}
