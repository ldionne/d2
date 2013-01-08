/**
 * This file contains unit tests for the segmentation graph construction.
 */

#include <d2/events/join_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/segment.hpp>
#include <d2/segmentation_graph.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"

#include <boost/graph/graphviz.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>


namespace {
    struct SegmentationGraphTest : ::testing::Test {
        std::vector<boost::variant<d2::StartEvent, d2::JoinEvent> > events;
        d2::SegmentationGraph graph;
        std::vector<d2::Thread> threads;
        boost::unordered_map<d2::Thread, d2::Segment> segment_of;
        std::vector<d2::Segment> segments;

        d2::StartEvent fake_start(d2::Thread parent, d2::Thread child) {
            d2::Segment parent_segment = segment_of[parent];
            d2::Segment new_parent_segment = new_segment();
            d2::Segment child_segment = new_segment();
            segment_of[child] = child_segment;
            segment_of[parent] = new_parent_segment;

            return d2::StartEvent(
                    parent_segment, new_parent_segment, child_segment);
        }

        d2::JoinEvent fake_join(d2::Thread parent, d2::Thread child) {
            d2::Segment parent_segment = segment_of[parent];
            d2::Segment child_segment = segment_of[child];
            d2::Segment new_parent_segment = new_segment();
            segment_of[parent] = new_parent_segment;
            segment_of.erase(child);

            return d2::JoinEvent(
                    parent_segment, new_parent_segment, child_segment);
        }

        d2::Segment new_segment() {
            d2::Segment current = segments.back() + 1;
            segments.push_back(current);
            return current;
        }

        void SetUp() {
            segments.push_back(d2::Segment()); // Initial segment.
            for (unsigned int i = 0; i < 1000; ++i)
                threads.push_back(d2::Thread(i));
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

TEST_F(SegmentationGraphTest, simple_start_and_join) {
    using namespace boost::assign;
    events +=
        fake_start(threads[0], threads[1]),
        fake_join(threads[0], threads[1])
    ;
    d2::build_segmentation_graph<>()(events, graph);
    ASSERT_EQ(segments.size(), num_vertices(graph));

    //      0   1   2   3
    // t0   o___o_______o
    // t1   \_______o__/

    ASSERT_EQ(4, num_vertices(graph));

    EXPECT_TRUE(happens_before(segments[0], segments[1], graph));
    EXPECT_TRUE(happens_before(segments[0], segments[2], graph));
    EXPECT_TRUE(happens_before(segments[0], segments[3], graph));

    EXPECT_FALSE(happens_before(segments[1], segments[2], graph));

    EXPECT_TRUE(happens_before(segments[0], segments[3], graph));
    EXPECT_TRUE(happens_before(segments[1], segments[3], graph));
    EXPECT_TRUE(happens_before(segments[2], segments[3], graph));
}
