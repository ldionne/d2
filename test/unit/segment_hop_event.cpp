/**
 * This file contains unit tests for the `SegmentHopEvent` event.
 */

#include "serialization_test.hpp"
#include <d2/events/segment_hop_event.hpp>
#include <d2/segment.hpp>
#include <d2/thread.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/utility/value_init.hpp>


namespace d2 {
namespace test {

struct SegmentHopEventTest {
    typedef SegmentHopEvent value_type;
    static boost::random::mt19937 gen;

    static value_type get_random_object() {
        // Thread ids are in [0, 10000]
        // Segment values are [initial segment, initial segment + 10000]
        boost::random::uniform_int_distribution<unsigned> dist(0, 10000);
        return value_type(Thread(dist(gen)), Segment() + dist(gen));
    }
};

boost::random::mt19937 SegmentHopEventTest::gen = boost::initialized_value;

INSTANTIATE_TYPED_TEST_CASE_P(SegmentHopEvent, SerializationTest, SegmentHopEventTest);

} // end namespace test
} // end namespace d2
