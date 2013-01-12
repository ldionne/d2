/**
 * This file contains unit tests for mixed event logging.
 */

#include <d2/events.hpp>
#include <d2/segment.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"


namespace {
    struct MixedEventTest : ::testing::Test {
        std::stringstream stream;
        std::vector<d2::Thread> threads;
        std::vector<d2::SyncObject> locks;
        std::vector<d2::Segment> segments;
        typedef boost::variant<
                    d2::AcquireEvent, d2::ReleaseEvent,
                    d2::StartEvent, d2::JoinEvent,
                    d2::SegmentHopEvent
                > Event;

        qi::typed_stream<d2::AcquireEvent> acquire;
        qi::typed_stream<d2::ReleaseEvent> release;
        qi::typed_stream<d2::StartEvent> start;
        qi::typed_stream<d2::JoinEvent> join;
        qi::typed_stream<d2::SegmentHopEvent> hop;

        void SetUp() {
            for (unsigned int i = 0; i < 100; ++i) {
                threads.push_back(d2::Thread(i));
                locks.push_back(d2::SyncObject(i));
                segments.push_back(d2::Segment() + i);
            }
        }
    };
} // end anonymous namespace

TEST_F(MixedEventTest, save_and_load_mixed_events) {
    using namespace boost::assign;
    std::vector<Event> saved;
    saved += d2::AcquireEvent(locks[0], threads[0]),
             d2::ReleaseEvent(locks[0], threads[0]),
             d2::StartEvent(segments[0], segments[1], segments[2]),
             d2::JoinEvent(segments[0], segments[1], segments[2]),
             d2::SegmentHopEvent(threads[0], segments[0]);

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<Event>(stream));
    EXPECT_TRUE(stream && "failed to save the mixed events");

    std::vector<Event> loaded;
    stream >> qi::match(*(acquire | release | start | join | hop), loaded);
    EXPECT_TRUE(stream && "failed to load the mixed events");

    ASSERT_EQ(saved, loaded);
}
