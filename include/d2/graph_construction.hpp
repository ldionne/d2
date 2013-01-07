/**
 * This file defines a thin interface to construct both graphs required during
 * the analysis.
 */

#ifndef D2_GRAPH_CONSTRUCTION_HPP
#define D2_GRAPH_CONSTRUCTION_HPP

#include <d2/filesystem_loader.hpp>
#include <d2/lock_graph.hpp>
#include <d2/segmentation_graph.hpp>

#include <string>


namespace d2 {
namespace detail {

// This is required because bind facilities can't take their arguments by
// non-const reference, so we can't build the graph by using them.
template <typename LockGraph>
struct PartialLockGraphBuilder {
    LockGraph& graph;

    explicit PartialLockGraphBuilder(LockGraph& graph)
        : graph(graph)
    { }

    typedef void result_type;

    template <typename Range>
    result_type operator()(Range const& range) const {
        build_lock_graph<>()(range, graph);
    }
};

} // end namespace detail

/**
 * Build the lock graph and the segmentation graph from the events at the
 * specified `path`.
 */
template <typename LockGraph, typename SegmentationGraph>
void build_graphs(std::string const& path, LockGraph& lock_graph,
                                           SegmentationGraph& seg_graph) {
    FilesystemLoader loader(path);
    build_segmentation_graph<>()(loader.process_events(), seg_graph);

    loader.for_each(detail::PartialLockGraphBuilder<LockGraph>(lock_graph));
}

} // end namespace d2

#endif // !D2_GRAPH_CONSTRUCTION_HPP
