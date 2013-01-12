/**
 * This file contains unit tests for the `AcquireEvent` event.
 */

#include "serialization_test.hpp"
#include <d2/events/acquire_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/value_init.hpp>


namespace d2 {
namespace test {

struct AcquireEventWithoutInfoTest {
    typedef AcquireEvent value_type;
    static boost::random::mt19937 gen;

    static value_type get_random_object() {
        // Lock and thread ids are in the range [0, 10000]
        boost::random::uniform_int_distribution<unsigned> dist(0, 10000);
        return value_type(SyncObject(dist(gen)), Thread(dist(gen)));
    }
};

boost::random::mt19937 AcquireEventWithoutInfoTest::gen = boost::initialized_value;

struct AcquireEventWithInfoTest : AcquireEventWithoutInfoTest {
    static value_type get_random_object() {
        value_type event = AcquireEventWithoutInfoTest::get_random_object();
        event.info.init_call_stack();
        return event;
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(AcquireEventWithInfo, SerializationTest, AcquireEventWithInfoTest);
INSTANTIATE_TYPED_TEST_CASE_P(AcquireEventWithoutInfo, SerializationTest, AcquireEventWithoutInfoTest);

} // end namespace test
} // end namespace d2
