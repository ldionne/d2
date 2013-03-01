/**
 * This file defines the segmentation graph data structure used during the
 * post-mortem program analysis.
 */

#ifndef D2_CORE_SEGMENTATION_GRAPH_HPP
#define D2_CORE_SEGMENTATION_GRAPH_HPP

#include <d2/segment.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/multi_index/identity.hpp>


namespace boost {
namespace graph {
    /**
     * This is to be able to refer to a vertex in the segmentation graph
     * using the `Segment` associated to it.
     */
    template <> struct internal_vertex_name<d2::Segment> {
        typedef multi_index::identity<d2::Segment> type;
    };
} // end namespace graph
} // end namespace boost

namespace d2 {
namespace core {
/**
 * Directed acyclic graph representing the order of starts and joins between
 * the threads of a program.
 */
typedef boost::adjacency_list<
            boost::vecS, boost::vecS, boost::directedS, Segment
        > SegmentationGraph;
} // end namespace core
} // end namespace d2

#endif // !D2_CORE_SEGMENTATION_GRAPH_HPP
