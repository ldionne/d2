/**
 * This file contains unit tests for the `AnyEvent` event.
 */

#include <d2/events/acquire_event.hpp>
#include <d2/events/any_event.hpp>
#include <d2/events/join_event.hpp>
#include <d2/events/release_event.hpp>
#include <d2/events/segment_hop_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/segment.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"

#include <algorithm>
#include <iterator>


namespace {
    struct AnyEventTest : ::testing::Test {
        std::stringstream stream;
        d2::StartEvent start;
        d2::JoinEvent join;
        d2::AcquireEvent acquire;
        d2::ReleaseEvent release;
        d2::SegmentHopEvent hop;

        void SetUp() {
            stream.unsetf(std::ios::skipws);
            d2::Thread thread((unsigned)0xDEADC0DE);
            d2::SyncObject lock((unsigned)0xBADBABE);
            d2::Segment s1, s2, s3;
            s1 += 100, s2 += 200, s3 += 300;

            start = d2::StartEvent(s1, s2, s3);
            join = d2::JoinEvent(s1, s2, s3);
            acquire = d2::AcquireEvent(lock, thread);
            release = d2::ReleaseEvent(lock, thread);
            hop = d2::SegmentHopEvent(thread, s1);
        }
    };
} // end anonymous namespace

TEST_F(AnyEventTest, save_and_load_a_single_initialized_any_event) {
    d2::AnyEvent saved = start;
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the any event");

    d2::AnyEvent loaded = d2::StartEvent();
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the any event");

    ASSERT_EQ(saved.as<d2::StartEvent>(), loaded.as<d2::StartEvent>());
}

namespace {
    bool event_equal(d2::AnyEvent const& a, d2::AnyEvent const& b) {
        typedef boost::mpl::vector<d2::StartEvent, d2::JoinEvent,
                                   d2::AcquireEvent, d2::ReleaseEvent,
                                   d2::SegmentHopEvent> Events;
        return d2::any_to_variant_of<Events>(a) ==
               d2::any_to_variant_of<Events>(b);
    }
} // end namespace detail

TEST_F(AnyEventTest, save_and_load_several_mixed_events) {
    using namespace boost::assign;
    std::vector<d2::AnyEvent> saved;
    saved += start, acquire, release, join, join, hop;

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<d2::AnyEvent>(stream));
    EXPECT_TRUE(stream && "failed to save the any events");

    std::vector<d2::AnyEvent> loaded;
    std::copy(std::istream_iterator<d2::AnyEvent>(stream),
              std::istream_iterator<d2::AnyEvent>(),
              std::back_inserter(loaded));
    EXPECT_TRUE(stream && "failed to load the any events");

    ASSERT_EQ(saved.size(), loaded.size());
    ASSERT_TRUE(std::equal(saved.begin(), saved.end(),
                           loaded.begin(), event_equal));
}
