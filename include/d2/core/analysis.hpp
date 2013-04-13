/*!
 * @file
 * This file defines the core graph analysis algorithm.
 */

#ifndef D2_CORE_ANALYSIS_HPP
#define D2_CORE_ANALYSIS_HPP

#include <d2/core/build_segmentation_graph.hpp> // for happens_before
#include <d2/detail/tiernan_all_cycles.hpp>
#include <d2/detail/vertex_to_edge_path.hpp>

#include <boost/assert.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/move/utility.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/ref.hpp>
#include <vector>


namespace d2 {
namespace analysis_detail {
//! Return whether two unordered containers have a non-empty intersection.
template <typename Unordered1, typename Unordered2>
bool unordered_intersects(Unordered1 const& a, Unordered2 const& b) {
    typedef typename Unordered1::const_iterator Iterator;
    typename Unordered2::const_iterator not_found(boost::end(b));
    Iterator elem(boost::begin(a)), last(boost::end(a));
    for (; elem != last; ++elem)
        if (b.find(*elem) != not_found)
            return true;
    return false;
}

/*!
 * @internal
 * Wrap a `BinaryFunction` to implement a visitor for the algorithm.
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

    /*!
     * Method called whenever a cycle is found.
     *
     * If the cycle respects certain conditions, calls the wrapped
     * `BinaryFunction` with a sequence of unspecified type containing the
     * edges in the cycle and a constant reference to the lock graph.
     *
     * @note The conditions are those that make a cycle a potential deadlock
     *       in the lock graph.
     */
    template <typename EdgePath>
    void cycle(EdgePath const& edge_path, LockGraph const& graph) const {
        EdgeLabelMap labels = get(boost::edge_bundle, graph);

        // These are the conditions for a cycle to be a valid
        // potential deadlock:
        // For any given pair of edges (e1, e2)
        typedef typename EdgePath::const_iterator EdgeIterator;
        EdgeIterator last = edge_path.end();
        for (EdgeIterator ei1 = edge_path.begin(); ei1 != last; ++ei1) {
            LockGraphEdgeDescriptor e1 = *ei1;
            for (EdgeIterator ei2 = edge_path.begin(); ei2 != last; ++ei2) {
                LockGraphEdgeDescriptor e2 = *ei2;

                if (e1 == e2)
                  continue;

                // The threads must differ.
                if (thread_of(labels[e1]) == thread_of(labels[e2]))
                  return;

                // The guard sets must not overlap.
                if (unordered_intersects(gatelocks_of(labels[e1]),
                                         gatelocks_of(labels[e2])))
                  return;

                // The segments must not be ordered.
                if (core::happens_before(labels[e1].s2, labels[e2].s1, sg_))
                  return;
            }
        }

        f_(edge_path, graph);
    }
};

/*!
 * @internal
 * Functor wrapping a cycle visitor.
 *
 * When the functor is called, the visitor's `cycle` method is called with
 * the same arguments.
 */
template <typename Visitor>
struct cycle_visitor_as_functor {
    explicit cycle_visitor_as_functor(Visitor const& visitor)
        : visitor_(visitor)
    { }

    explicit cycle_visitor_as_functor(BOOST_RV_REF(Visitor) visitor)
        : visitor_(boost::move(visitor))
    { }

    template <typename Cycle, typename Graph>
    void operator()(BOOST_FWD_REF(Cycle) c, BOOST_FWD_REF(Graph) g) const {
        visitor_.cycle(boost::forward<Cycle>(c), boost::forward<Graph>(g));
    }

private:
    Visitor visitor_;
};

/*!
 * @internal
 * Transform a cycle of vertex descriptors into one or more cycle of edge
 * descriptors.
 *
 * The wrapped cycle visitor is called with each cycle thus generated.
 */
template <typename Visitor>
struct vertex_to_edge_cycle {
    explicit vertex_to_edge_cycle(Visitor const& visitor)
        : visitor_(visitor)
    { }

    template <typename VertexPath, typename Graph>
    void cycle(VertexPath vpath, Graph const& graph) const {
        BOOST_ASSERT_MSG(vpath.size() >= 2,
            "a cycle with less than two vertices is impossible, "
            "since we're not considering self loops");

        // Close the cycle so we can transform it to a cycle of edges.
        vpath.push_back(vpath.front());

        // Call the visitor with all the edge paths.
        detail::make_vertex_to_edge_path(
            cycle_visitor_as_functor<Visitor>(visitor_))(vpath, graph);
    }

private:
    Visitor visitor_;
};

/*!
 * Analyze the lock graph and the segmentation graph to determine whether the
 * program execution represented by them contains a deadlock. `f` is called
 * whenever a potential deadlock is detected.
 */
template <typename LockGraph, typename SegmentationGraph, typename F>
void analyze(LockGraph const& lg, SegmentationGraph const& sg, F const& f) {
    typedef CycleVisitor<LockGraph, SegmentationGraph, F> EdgeCycleVisitor;
    typedef vertex_to_edge_cycle<EdgeCycleVisitor> VertexCycleVisitor;
    EdgeCycleVisitor edge_visitor(sg, f);
    VertexCycleVisitor vertex_visitor(edge_visitor);

    boost::tiernan_all_cycles(lg, vertex_visitor);
}
} // end namespace analysis_detail

namespace core {
    using analysis_detail::analyze;
}
} // end namespace d2

#endif // !D2_CORE_ANALYSIS_HPP
