/**
 * This file contains unit tests for the `AcquireEvent` event.
 */

#include "../serialization_test.hpp"
#include <d2/events/acquire_event.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/value_init.hpp>


namespace d2 {
namespace test {

namespace detail {
// Same as an AcquireEvent, but the operator== also checks for the
// lock debug info to be the same.
struct MockAcquireEvent : AcquireEvent {
    MockAcquireEvent() { }

    MockAcquireEvent(LockId const& lock, ThreadId const& thread)
        : AcquireEvent(lock, thread)
    { }

    friend bool operator==(MockAcquireEvent const& a,
                           MockAcquireEvent const& b) {
        return static_cast<AcquireEvent const&>(a) ==
               static_cast<AcquireEvent const&>(b) &&
               a.info == b.info;
    }
};
} // end namespace detail

struct AcquireEventWithoutInfoTest {
    typedef detail::MockAcquireEvent value_type;
    static boost::random::mt19937 gen;

    static value_type get_random_object() {
        // Lock and thread ids are in the range [0, 10000]
        boost::random::uniform_int_distribution<unsigned> dist(0, 10000);
        return value_type(LockId(dist(gen)), ThreadId(dist(gen)));
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
