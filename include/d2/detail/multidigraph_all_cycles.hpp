/**
 * This file defines the `multidigraph_all_cycles` algorithm to find all the
 * cycles in a directed (multi)graph.
 */

#ifndef D2_DETAIL_MULTIDIGRAPH_ALL_CYCLES_HPP
#define D2_DETAIL_MULTIDIGRAPH_ALL_CYCLES_HPP

#include <d2/detail/tiernan_all_cycles.hpp>

#include <boost/assert.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/tuple/tuple.hpp> // for boost::tie
#include <vector>


namespace d2 {
namespace multidigraph_all_cycles_detail {
/**
 * Visitor wrapper transforming a cycle of the form
 * @code
 *      (vertex descriptor 1, vertex descriptor 2, ..., vertex descriptor N)
 * @endcode
 * to several cycles of the form
 * @code
 *      (edge descriptor 1 2, edge descriptor 2 3, ..., edge descriptor N-1 N)
 * @endcode
 *
 * For every possible cycle of the second form, the wrapped visitor is called
 * with the corresponding cycle.
 */
template <typename Graph, typename Visitor>
class VerticesToMultigraphEdges {
    typedef boost::graph_traits<Graph> Traits;
    typedef typename Traits::vertex_descriptor VertexDescriptor;
    typedef typename Traits::edge_descriptor EdgeDescriptor;
    typedef typename Traits::out_edge_iterator OutEdgeIterator;

    Visitor visitor_;

    template <typename EdgePath, typename Iterator>
    void visit_all_edges(EdgePath& path, Iterator first, Iterator last,
                                                         Graph const& graph) {
        VertexDescriptor u = *first++;
        if (first == last) {
            visitor_.cycle(path, graph);
            return;
        }
        VertexDescriptor v = *first;

        OutEdgeIterator oe, oe_end;
        // For all out edges of u, we only consider those whose target is v.
        for (boost::tie(oe, oe_end) = out_edges(u, graph); oe != oe_end; ++oe) {
            EdgeDescriptor e = *oe;
            if (target(e, graph) != v)
                continue;

            path.push_back(e);
            visit_all_edges(path, first, last, graph);
            path.pop_back();
        }
    }

public:
    explicit VerticesToMultigraphEdges(Visitor const& visitor)
        : visitor_(visitor)
    { }

    template <typename VertexPath>
    void cycle(VertexPath vp, Graph const& graph) {
        // complete the cycle, which is implicit when using tiernan_all_cycles
        vp.push_back(vp.front());

        BOOST_ASSERT_MSG(vp.size() >= 2,
            "a cycle can't exist with fewer than two vertices "
            "in it (we're not considering self-loops)");

        std::vector<EdgeDescriptor> ep;
        ep.reserve(vp.size() - 1); // we need n-1 edges to chain n vertices
        visit_all_edges(ep, vp.begin(), vp.end(), graph);
    }
};

/**
 * Algorithm finding all the cycles in a directed multigraph.
 *
 * The visitor is called with cycles of the form:
 * @code
 *      (edge descriptor 1, edge descriptor 2, ..., edge descriptor N)
 * @endcode
 *
 * @note The algorithm also works with digraphs that are not multigraphs.
 */
template <typename Graph, typename Visitor>
void multidigraph_all_cycles(Graph const& graph, Visitor const& visitor) {
    typedef VerticesToMultigraphEdges<Graph, Visitor> Wrapper;
    Wrapper wrapper(visitor);
    boost::tiernan_all_cycles(graph, wrapper);
}
} // end namespace multidigraph_all_cycles_detail

namespace detail {
    using multidigraph_all_cycles_detail::multidigraph_all_cycles;
}
} // end namespace d2

#endif // !D2_DETAIL_MULTIDIGRAPH_ALL_CYCLES_HPP
