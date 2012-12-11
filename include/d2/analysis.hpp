/**
 * This file defines the core graph analysis algorithm.
 */

#ifndef D2_ANALYSIS_HPP
#define D2_ANALYSIS_HPP

#include <d2/types.hpp>

#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <vector>


namespace boost {
namespace graph {
    /**
     * Return whether vertex `v` is reachable from vertex `u`.
     * @note This is an extension to the existing `is_reachable`, which
     *       requires passing a property map.
     */
    template <typename Graph>
    bool is_reachable(typename graph_traits<Graph>::vertex_descriptor u,
                      typename graph_traits<Graph>::vertex_descriptor v,
                      Graph const& g) {
        one_bit_color_map<> map(num_vertices(g));
        return is_reachable(u, v, g, map);
    }
} // end namespace graph
} // end namespace boost


namespace d2 {
namespace detail {

/**
 * Return whether two unordered containers have a non-empty intersection.
 */
template <typename Unordered1, typename Unordered2>
bool unordered_intersects(Unordered1 const& a, Unordered2 const& b) {
    typedef typename Unordered1::const_iterator Iterator;
    typename Unordered2::const_iterator not_found(boost::end(b));
    Iterator elem(boost::begin(a)), last(boost::end(a));
    for (; elem != last; ++elem)
        if (b.find(*elem) == not_found)
            return false;
    return true;
}

/**
 * Wrap a `BinaryFunction` to implement a visitor for the goodlock algorithm.
 * @todo If we use an adjacency_matrix to store the segmentation graph, we
 *       should compute its transitive closure to reduce the complexity of
 *       the happens-before relation.
 */
template <typename LockGraph, typename SegmentationGraph, typename Function>
class cycle_visitor {
    typedef typename boost::graph_traits<LockGraph>::edge_descriptor
                                                    LockGraphEdgeDescriptor;
    typedef typename boost::graph_traits<LockGraph>::vertex_descriptor
                                                    LockGraphVertexDescriptor;
    typedef std::vector<LockGraphEdgeDescriptor> EdgePath;

    // Property map to access the edge labels of the lock graph.
    typedef typename boost::property_map<LockGraph,
                               boost::edge_bundle_t>::const_type EdgeLabelMap;
    SegmentationGraph const& sg_;
    Function& f_;

    /**
     * Transform a path of vertices [u, v, w] to a path of edges
     * [(u, v), (v, w)].
     * @todo Use an iterator computing the edges on the fly instead.
     */
    template <typename VertexPath>
    static EdgePath to_edge_path(VertexPath const& p, LockGraph const& lg) {
        typename VertexPath::const_iterator first(boost::begin(p)),
                                            last(boost::end(p));
        EdgePath edge_path;
        if (first == last)
            return edge_path;
        for (LockGraphVertexDescriptor u = *first++; first != last;
                                                                u = *first++)
            edge_path.push_back(edge(u, *first, lg).first);
        return edge_path;
    }

    /**
     * Return whether segment `u` happens before segment `v` according to
     * the segmentation graph.
     */
    bool happens_before(segment u, segment v) const {
        return boost::graph::is_reachable(u, v, sg_);
    }

public:
    cycle_visitor(SegmentationGraph const& sg, Function& f)
        : sg_(sg), f_(f)
    { }

    /**
     * Function called by `tiernan_all_cycles` whenever a cycle is found.
     * It calls the wrapped function with a sequence containing the edges
     * in the cycle and a constant reference to the lock graph, but only if
     * the cycle respects certain conditions, i.e. if the cycle represents a
     * deadlock in the lock graph.
     */
    template <typename VertexPath>
    void cycle(VertexPath const& vertex_path, LockGraph const& graph) const {
        EdgePath edge_path = to_edge_path(vertex_path, graph);

        // For any given pair of edges (e1, e2)
        EdgeLabelMap labels = get(boost::edge_bundle, graph);
        BOOST_FOREACH(LockGraphEdgeDescriptor e1, edge_path) {
            BOOST_FOREACH(LockGraphEdgeDescriptor e2, edge_path) {
                if (e1 == e2) continue;
                else if (!(

                    // The threads must differ.
                    labels[e1].t != labels[e2].t &&

                    // The guard sets must not overlap.
                    !unordered_intersects(labels[e1].g, labels[e2].g) &&

                    // The segments must not be ordered.
                    !happens_before(labels[e1].s2, labels[e2].s1)

                )) return;
            }
        }

        f_(edge_path, graph);
    }
};

} // end namespace detail

/**
 * Analyze the lock graph and the segmentation graph to determine whether the
 * program execution represented by them contains a deadlock. `f` is called
 * whenever a potential deadlock is detected.
 * @see `detail::cycle_visitor` for more details.
 */
template <typename LockGraph, typename SegmentationGraph, typename Function>
void analyze(LockGraph const& lg, SegmentationGraph const& sg, Function f) {
    BOOST_CONCEPT_ASSERT((LockGraphConcept<LockGraph>));

    detail::cycle_visitor<LockGraph, SegmentationGraph, Function>
                                                            visitor(sg, f);
    boost::tiernan_all_cycles(lg, visitor);
}

} // end namespace d2

#endif // !D2_ANALYSIS_HPP
