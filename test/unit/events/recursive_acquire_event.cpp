/**
 * This file contains unit tests for the `RecursiveAcquireEvent` event.
 */

#include "../serialization_test.hpp"
#include <d2/events/recursive_acquire_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/value_init.hpp>


namespace d2 {
namespace test {

namespace detail {
// Same as a RecursiveAcquireEvent, but the operator== also checks for the
// lock debug info to be the same.
struct MockRecursiveAcquireEvent : RecursiveAcquireEvent {
    MockRecursiveAcquireEvent() { }

    MockRecursiveAcquireEvent(SyncObject const& lock, Thread const& thread)
        : RecursiveAcquireEvent(lock, thread)
    { }

    friend bool operator==(MockRecursiveAcquireEvent const& a,
                           MockRecursiveAcquireEvent const& b) {
        return static_cast<RecursiveAcquireEvent const&>(a) ==
               static_cast<RecursiveAcquireEvent const&>(b) &&
               a.info == b.info;
    }
};
} // end namespace detail

struct RecursiveAcquireEventWithoutInfoTest {
    typedef detail::MockRecursiveAcquireEvent value_type;
    static boost::random::mt19937 gen;

    static value_type get_random_object() {
        // Lock and thread ids are in the range [0, 10000]
        boost::random::uniform_int_distribution<unsigned> dist(0, 10000);
        return value_type(SyncObject(dist(gen)), Thread(dist(gen)));
    }
};

boost::random::mt19937 RecursiveAcquireEventWithoutInfoTest::gen = boost::initialized_value;

struct RecursiveAcquireEventWithInfoTest : RecursiveAcquireEventWithoutInfoTest {
    static value_type get_random_object() {
        value_type event = RecursiveAcquireEventWithoutInfoTest::get_random_object();
        event.info.init_call_stack();
        return event;
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(RecursiveAcquireEventWithInfo, SerializationTest, RecursiveAcquireEventWithInfoTest);
INSTANTIATE_TYPED_TEST_CASE_P(RecursiveAcquireEventWithoutInfo, SerializationTest, RecursiveAcquireEventWithoutInfoTest);

} // end namespace test
} // end namespace d2
