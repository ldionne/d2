/**
 * This file defines the `non_equivalent_cycles` algorithm to compute cycles
 * in a directed graph.
 */

#ifndef D2_CORE_NON_EQUIVALENT_CYCLES_HPP
#define D2_CORE_NON_EQUIVALENT_CYCLES_HPP

#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/unordered_map.hpp>
#include <deque>


namespace d2 {
namespace non_equivalent_cycles_detail {
template <typename Visitor, typename Graph, typename Cycle>
class Wrapper : public boost::dfs_visitor<> {
    Visitor visitor_;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    BOOST_CONCEPT_ASSERT((boost::FrontInsertionSequenceConcept<Cycle>));

    typedef boost::unordered_map<Vertex, Edge> PredecessorMap;
    PredecessorMap predecessors_;

public:
    Wrapper(Visitor const& v, Graph const&)
        : visitor_(v)
    { }

    void tree_edge(Edge e, Graph const& g) {
        predecessors_[target(e, g)] =  e;
    }

    void back_edge(Edge e, Graph const& g) {
        // Using the predecessor map maintained by the
        // edge_predecessor_recorder, we create a path of the form:
        // (u, v) (v, w) (w, x) ...
        // Representing the edges forming the cycle. We then call the adapted
        // visitor with that path, which is much easier to manipulate.
        Cycle cycle;
        cycle.push_front(e);
        typedef typename PredecessorMap::const_iterator PredecessorIterator;
        while (true) {
            PredecessorIterator it = predecessors_.find(source(e, g));
            if (it == boost::end(predecessors_))
                break;

            cycle.push_front(e = it->second);
        }

        visitor_.cycle(cycle, g);
    }
};

/**
 * Algorithm to compute all the non-equivalent cycles in a graph.
 *
 * Two cycles are equivalent if and only if one is equal to some rotation
 * of the other.
 */
template <typename Graph, typename Visitor>
void non_equivalent_cycles(Graph const& g, Visitor const& visitor) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef std::deque<Edge> Cycle;

    if (num_vertices(g) == 0)
        return;

    Wrapper<Visitor, Graph, Cycle> wrapper(visitor, g);
    Vertex first_vertex = *vertices(g).first;
    boost::depth_first_search(g,
        boost::root_vertex(first_vertex).visitor(wrapper));
}
} // end namespace non_equivalent_cycles_detail

namespace core {
    using non_equivalent_cycles_detail::non_equivalent_cycles;
}
} // end namespace d2

#endif // !D2_CORE_NON_EQUIVALENT_CYCLES_HPP
