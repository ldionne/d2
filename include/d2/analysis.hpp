/**
 * This file defines the core graph analysis algorithm.
 */

#ifndef D2_ANALYSIS_HPP
#define D2_ANALYSIS_HPP

#include <d2/types.hpp>

#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/unordered_map.hpp>
#include <deque>
#include <set>
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

// Note: You can enable/disable this to debug the algorithm.
#define D2_DEBUG_ALL_CYCLES_DUMB(statement) do { } while (false)
// #define D2_DEBUG_ALL_CYCLES_DUMB(statement) do { statement; } while (false)

/**
 * Wrapper visitor for use within the `all_cycles_dumb` algorith. It allows
 * the wrapped visitor to keep the same interface as for `tiernan_all_cycles`.
 */
template <typename Adapted, typename Graph>
class all_cycles_dumb_wrapper : public boost::dfs_visitor<> {
    Adapted visitor_;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

    typedef boost::unordered_map<Vertex, Edge> PredecessorMap;
    PredecessorMap predecessors_;
    std::set<std::deque<Edge> >& seen_cycles;

public:
    explicit all_cycles_dumb_wrapper(Adapted const& v, Graph const&,
                                     std::set<std::deque<Edge> >& seen)
        : visitor_(v), seen_cycles(seen)
    { }

    void tree_edge(Edge e, Graph const& g) {
        D2_DEBUG_ALL_CYCLES_DUMB(std::cout <<
            "tree edge: " << e << '\n' <<
            "set predecessor of " << target(e, g) << " to " << e << '\n');
        predecessors_[target(e, g)] =  e;
    }

    void back_edge(Edge e, Graph const& g) {
        D2_DEBUG_ALL_CYCLES_DUMB(std::cout <<
            "back edge: " << e << '\n' <<
            "making sure " << source(e, g) << " has a predecessor\n");
        BOOST_ASSERT_MSG(
            predecessors_.find(source(e,g)) != boost::end(predecessors_),
            "the predecessor edge of the source of the current edge is not "
            "defined, something's wrong");

        // Using the predecessor map maintained by the
        // edge_predecessor_recorder, we create a path of the form:
        // (u, v) (v, w) (w, x) ...
        // Representing the edges forming the cycle. We then call the adapted
        // visitor with that path, which is much easier to manipulate.
        std::deque<Edge> cycle;
        cycle.push_front(e);
        typedef typename PredecessorMap::const_iterator PredecessorIterator;
        while (true) {
            PredecessorIterator it = predecessors_.find(source(e, g));
            if (it == boost::end(predecessors_))
                break;

            cycle.push_front(e = it->second);
        }

        // Note: Since the all_cycles_dumb algorithm starts a depth first
        //       traversal at _each_ vertex, we may encounter the same cycle
        //       many times. To avoid calling the visitor redundantly, we
        //       make sure we have not seen the cycle before.
        if (seen_cycles.insert(cycle).second)
            visitor_.cycle(cycle, g);
    }
};

/**
 * Incredibly bad algorithm to compute all the cycles in a graph. It starts
 * a depth-first search at every vertex and calls the visitor when it finds
 * a cycle.
 */
template <typename Graph, typename Visitor>
void all_cycles_dumb(Graph const& g, Visitor const& vis) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;

    std::set<std::deque<Edge> > seen_cycles;

    all_cycles_dumb_wrapper<Visitor, Graph> wrapper(vis, g, seen_cycles);
    BOOST_FOREACH(Vertex u, vertices(g)) {
        boost::depth_first_search(g, boost::root_vertex(u).visitor(wrapper));
    }
}

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
     * Return whether segment `u` happens before segment `v` according to
     * the segmentation graph.
     */
    bool happens_before(Segment u, Segment v) const {
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
    template <typename EdgePath>
    void cycle(EdgePath const& edge_path, LockGraph const& graph) const {
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

    detail::cycle_visitor<LockGraph, SegmentationGraph, Function> vis(sg, f);
    detail::all_cycles_dumb(lg, vis);
}

} // end namespace d2

#endif // !D2_ANALYSIS_HPP
