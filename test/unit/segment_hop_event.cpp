/**
 * This file contains unit tests for the `SegmentHopEvent` event.
 */

#include <d2/events/segment_hop_event.hpp>
#include <d2/segment.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"


namespace {
    struct SegmentHopEventTest : ::testing::Test {
        std::stringstream stream;
        d2::Thread thread;
        d2::Segment segment;

        void SetUp() {
            thread = d2::Thread((unsigned)0xBADC0FFEE);
            segment += 0xF00BA12;
            stream.unsetf(std::ios::skipws);
        }
    };
} // end anonymous namespace

TEST_F(SegmentHopEventTest, save_and_load_a_single_hop_event) {
    d2::SegmentHopEvent saved(thread, segment);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the hop event");

    d2::SegmentHopEvent loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the hop event");

    ASSERT_EQ(saved, loaded);
}

TEST_F(SegmentHopEventTest, save_and_load_several_hop_events) {
    using namespace boost::assign;
    std::vector<d2::SegmentHopEvent> saved;
    saved += d2::SegmentHopEvent(thread, segment),
             d2::SegmentHopEvent(thread, segment),
             d2::SegmentHopEvent(thread, segment);

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<d2::SegmentHopEvent>(stream));
    EXPECT_TRUE(stream && "failed to save the hop events");

    std::vector<d2::SegmentHopEvent> loaded;
    std::copy(std::istream_iterator<d2::SegmentHopEvent>(stream),
              std::istream_iterator<d2::SegmentHopEvent>(),
              std::back_inserter(loaded));
    EXPECT_TRUE(stream.eof() && "failed to load the hop events");

    ASSERT_EQ(saved, loaded);
}
