/**
 * This file implements the construction of the graphs.
 */

#define D2_SOURCE
#include <d2/core/build_lock_graph.hpp>
#include <d2/core/build_segmentation_graph.hpp>
#include <d2/core/events.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/core/segmentation_graph.hpp>
#include <d2/core/sync_skeleton.hpp>

#include <dyno/istream_iterator.hpp>


namespace d2 {
namespace sync_skeleton_detail {
extern void parse_and_build_seg_graph(StreamType& is,
                                      core::SegmentationGraph& graph) {
    typedef dyno::istream_iterator<
                StreamType, core::events::non_thread_specific
            > Iterator;

    Iterator first(is), last;
    core::build_segmentation_graph<true>(first, last, graph);
}

extern void parse_and_build_lock_graph(StreamType& is,
                                       core::LockGraph& graph) {
    typedef dyno::istream_iterator<
                StreamType, core::events::thread_specific
            > Iterator;

    Iterator first(is), last;
    core::build_lock_graph<true>(first, last, graph);
}
} // end namespace sync_skeleton_detail
} // end namespace d2
