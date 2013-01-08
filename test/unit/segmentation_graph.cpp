/**
 * This file contains unit tests for the segmentation graph construction.
 */

#include <d2/events/acquire_event.hpp>
#include <d2/events/exceptions.hpp>
#include <d2/events/join_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/segment.hpp>
#include <d2/segmentation_graph.hpp>
#include "test_base.hpp"

#include <boost/graph/graphviz.hpp>
#include <boost/variant.hpp>


namespace {
    struct SegmentationGraphTest : ::testing::Test {
        std::vector<boost::variant<d2::StartEvent, d2::JoinEvent> > events;
        d2::SegmentationGraph graph;
        std::vector<d2::Segment> segments;

        void SetUp() {
            for (unsigned int i = 0; i < 1000; ++i)
                segments.push_back(d2::Segment() + i);
        }

        void TearDown() {
            if (HasFailure()) {
                std::cout << "Test failed, printing the segmentation graph:\n";
                boost::write_graphviz(std::cout, graph);
            }
        }
    };
} // end anonymous namespace

TEST_F(SegmentationGraphTest, no_events_create_empty_graph) {
    d2::build_segmentation_graph<>()(events, graph);

    ASSERT_EQ(0, num_vertices(graph));
}

TEST_F(SegmentationGraphTest, test_one_start_event_adds_right_edges) {
    using namespace boost::assign;
    events += d2::StartEvent(segments[0], segments[1], segments[2]);

    //      0   1   2
    // t0   o___o
    // t1   \_______o
    d2::build_segmentation_graph<>()(events, graph);
    ASSERT_EQ(3, num_vertices(graph));

    EXPECT_TRUE(happens_before(segments[0], segments[1], graph));
    EXPECT_TRUE(happens_before(segments[0], segments[2], graph));

    EXPECT_FALSE(happens_before(segments[1], segments[2], graph));
    EXPECT_FALSE(happens_before(segments[2], segments[1], graph));
}

TEST_F(SegmentationGraphTest, simple_start_and_join) {
    using namespace boost::assign;
    events +=
        d2::StartEvent(segments[0], segments[1], segments[2]),
        d2::JoinEvent(segments[1], segments[3], segments[2])
    ;

    //      0   1   2   3
    // t0   o___o_______o
    // t1   \_______o__/
    d2::build_segmentation_graph<>()(events, graph);
    ASSERT_EQ(4, num_vertices(graph));

    EXPECT_TRUE(happens_before(segments[0], segments[1], graph));
    EXPECT_TRUE(happens_before(segments[0], segments[2], graph));
    EXPECT_TRUE(happens_before(segments[0], segments[3], graph));

    EXPECT_FALSE(happens_before(segments[1], segments[2], graph));

    EXPECT_TRUE(happens_before(segments[0], segments[3], graph));
    EXPECT_TRUE(happens_before(segments[1], segments[3], graph));
    EXPECT_TRUE(happens_before(segments[2], segments[3], graph));
}

TEST_F(SegmentationGraphTest, throws_on_unexpected_event_when_told_to) {
    using namespace boost::assign;
    typedef boost::variant<d2::StartEvent, d2::JoinEvent, d2::AcquireEvent>
                                                                    Events;
    std::vector<Events> events;
    events +=
        d2::StartEvent(segments[0], segments[1], segments[2]),
        d2::AcquireEvent(),
        d2::JoinEvent(segments[1], segments[3], segments[2])
    ;

    // This won't ignore unexpected events.
    d2::build_segmentation_graph<false> build;
    ASSERT_THROW(build(events, graph), d2::UnexpectedEventException);
}

TEST_F(SegmentationGraphTest, has_strong_guarantee_when_first_event_is_not_a_start_event) {
    using namespace boost::assign;
    events +=
        // Note: join comes before start.
        d2::JoinEvent(segments[1], segments[3], segments[2]),
        d2::StartEvent(segments[0], segments[1], segments[2])
    ;

    // It should throw because the first event is not a StartEvent as expected.
    d2::build_segmentation_graph<> build;
    ASSERT_THROW(build(events, graph), d2::UnexpectedEventException);

    // It should leave the graph untouched.
    ASSERT_EQ(0, num_vertices(graph));
}
