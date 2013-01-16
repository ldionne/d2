/**
 * This file defines the core graph analysis algorithm.
 */

#ifndef D2_ANALYSIS_HPP
#define D2_ANALYSIS_HPP

#include <d2/detail/all_cycles.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>


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
 *
 * @internal If we use an adjacency_matrix to store the segmentation graph, we
 *           should compute its transitive closure to reduce the complexity of
 *           the happens-before relation.
 */
template <typename LockGraph, typename SegmentationGraph, typename Function>
class CycleVisitor {
    typedef boost::graph_traits<LockGraph> GraphTraits;
    typedef typename GraphTraits::edge_descriptor LockGraphEdgeDescriptor;

    // Property map to access the edge labels of the lock graph.
    typedef typename boost::property_map<
                LockGraph, boost::edge_bundle_t
            >::const_type EdgeLabelMap;

    SegmentationGraph const& sg_;
    Function f_;

public:
    CycleVisitor(SegmentationGraph const& sg, Function const& f)
        : sg_(sg), f_(f)
    { }

    /**
     * Method called whenever a cycle is found.
     *
     * If the cycle respects certain conditions, calls the wrapped
     * `BinaryFunction` with a sequence of unspecified type containing the
     * edges in the cycle and a constant reference to the lock graph.
     *
     * @note The conditions are those that make a cycle a potential deadlock
     *       in the lock graph.
     * @internal This must not be const in order to allow non-const functors
     *           to be called.
     */
    template <typename EdgePath>
    void cycle(EdgePath const& edge_path, LockGraph const& graph) {
        // These are the conditions for a cycle to be a valid
        // potential deadlock:

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
                    !happens_before(labels[e1].s2, labels[e2].s1, sg_)

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
 *
 * @see `detail::CycleVisitor` for more details.
 */
template <typename LockGraph, typename SegmentationGraph, typename Function>
void analyze(LockGraph const& lg, SegmentationGraph const& sg,
             Function const& f) {
    detail::CycleVisitor<LockGraph, SegmentationGraph, Function> vis(sg, f);
    detail::all_cycles(lg, vis);
}

} // end namespace d2

#endif // !D2_ANALYSIS_HPP
