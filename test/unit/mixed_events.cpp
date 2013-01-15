/**
 * This file contains unit tests for mixed event saving/loading.
 */

#include <d2/events.hpp>
#include <d2/segment.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <algorithm>
#include <boost/assign.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/variant.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>


namespace karma = boost::spirit::karma;
namespace qi = boost::spirit::qi;

namespace std {
    // We need that for ASSERT_EQ(vector, vector)
    template <typename T, typename A>
    ostream& operator<<(ostream& os, vector<T, A> const& v) {
        os << karma::format('(' << karma::stream % ", " << ')', v);
        return os;
    }
} // end namespace std

namespace d2 {
namespace test {

struct MixedEventTest : ::testing::Test {
    typedef boost::mpl::vector<
                AcquireEvent,
                ReleaseEvent,
                StartEvent,
                JoinEvent,
                SegmentHopEvent,
                RecursiveAcquireEvent,
                RecursiveReleaseEvent
            > EventTypes;

    typedef boost::make_variant_over<EventTypes>::type Event;

    std::stringstream stream;
    std::vector<Thread> threads;
    std::vector<SyncObject> locks;
    std::vector<Segment> segments;

    qi::typed_stream<AcquireEvent> acquire;
    qi::typed_stream<ReleaseEvent> release;
    qi::typed_stream<StartEvent> start;
    qi::typed_stream<JoinEvent> join;
    qi::typed_stream<SegmentHopEvent> hop;
    qi::typed_stream<RecursiveAcquireEvent> rec_acquire;
    qi::typed_stream<RecursiveReleaseEvent> rec_release;

    void SetUp() {
        for (unsigned int i = 0; i < 1000; ++i) {
            threads.push_back(Thread(i));
            locks.push_back(SyncObject(i));
            segments.push_back(Segment() + i);
        }
    }
};


TEST_F(MixedEventTest, save_and_load_mixed_events) {
    using namespace boost::assign;
    std::vector<Event> saved;
    AcquireEvent some_acquire(locks[84], threads[45]);
    some_acquire.info.init_call_stack();

    RecursiveAcquireEvent some_rec_acquire(locks[84], threads[45]);
    some_rec_acquire.info.init_call_stack();

    saved += AcquireEvent(locks[3], threads[8]),
             ReleaseEvent(locks[0], threads[0]),
             RecursiveReleaseEvent(locks[0], threads[0]),
             some_rec_acquire,
             some_acquire,
             some_rec_acquire,
             StartEvent(segments[0], segments[1], segments[2]),
             JoinEvent(segments[0], segments[1], segments[2]),
             SegmentHopEvent(threads[0], segments[0]),
             some_acquire,
             RecursiveReleaseEvent(locks[19], threads[20]),
             some_acquire;

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<Event>(stream));
    EXPECT_TRUE(stream) << "failed to save the mixed events";

    std::vector<Event> loaded;
    std::string string(stream.str());
    std::string::const_iterator first(string.begin()), last(string.end());
    bool success =
        qi::parse(first, last,
            *(acquire | release | start | join | hop | rec_acquire | rec_release)
        , loaded);

    EXPECT_TRUE(success) << "failed to load the mixed events";
    EXPECT_TRUE(first == last) << "failed to load the mixed events";

    ASSERT_EQ(saved, loaded);
}

} // end namespace test
} // end namespace d2
