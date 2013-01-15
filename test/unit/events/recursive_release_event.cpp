/**
 * This file contains unit tests for the `RecursiveReleaseEvent` event.
 */

#include "../serialization_test.hpp"
#include <d2/events/recursive_release_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/value_init.hpp>


namespace d2 {
namespace test {

struct RecursiveReleaseEventTest {
    typedef RecursiveReleaseEvent value_type;
    static boost::random::mt19937 gen;

    static value_type get_random_object() {
        // Lock and thread ids are in the range [0, 10000]
        boost::random::uniform_int_distribution<unsigned> dist(0, 10000);
        return value_type(SyncObject(dist(gen)), Thread(dist(gen)));
    }
};

boost::random::mt19937 RecursiveReleaseEventTest::gen = boost::initialized_value;

INSTANTIATE_TYPED_TEST_CASE_P(RecursiveReleaseEvent, SerializationTest, RecursiveReleaseEventTest);

} // end namespace test
} // end namespace d2
