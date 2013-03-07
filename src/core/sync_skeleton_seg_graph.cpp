/**
 * This file implements the `build_segmentation_graph` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/build_segmentation_graph.hpp>
#include <d2/core/events.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/detail/decl.hpp>

#include <dyno/istream_iterator.hpp>


namespace d2 {
namespace synchronization_skeleton_detail {
D2_DECL void
synchronization_skeleton::build_segmentation_graph(Stream& stream) {
    typedef dyno::istream_iterator<
                Stream, core::events::non_thread_specific
            > Iterator;

    Iterator first(stream), last;
    static bool const ignore_other_events = true;
    core::build_segmentation_graph<ignore_other_events>(first, last, sg_);
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
