/**
 * This file implements the `feed_lock_graph` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/build_lock_graph.hpp>
#include <d2/core/events.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/detail/decl.hpp>

#include <dyno/istream_iterator.hpp>


namespace d2 {
namespace synchronization_skeleton_detail {
D2_DECL void synchronization_skeleton::feed_lock_graph(Stream& stream) {
    typedef dyno::istream_iterator<
                Stream, core::events::thread_specific
            > Iterator;

    Iterator first(stream), last;
    static bool const ignore_unrelated_events = true;
    core::build_lock_graph<ignore_unrelated_events>(first, last, lg_);
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
