/**
 * This file defines the `all_cycles` algorithm to find all the
 * cycles in a directed graph.
 */

#ifndef D2_CORE_ALL_CYCLES_HPP
#define D2_CORE_ALL_CYCLES_HPP

#include <boost/assert.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/unordered_map.hpp>
#include <deque>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>


namespace d2 {
namespace all_cycles_detail {
// Note: You can enable/disable this to debug the algorithm. Some information
//       will be printed to stdout.
#define D2_IMPL_DEBUG_ALL_CYCLES(statement) do { } while (false)
// #define D2_IMPL_DEBUG_ALL_CYCLES(statement) do { statement; } while (false)

/**
 * Wrapper visitor for use within the `all_cycles` algorithm.
 *
 * Its use is twofold:
 *  Keep track of the cycles seen so far, so the `all_cycles` algorithm can
 *  start its dfs only on vertices involved in a cycle.
 *
 *  Call a visitor complying to the `boost::tiernan_all_cycles` visitor
 *  interface on every cycle encountered.
 */
template <typename Visitor, typename Graph, typename SeenCycles>
class AllCyclesWrapper : public boost::dfs_visitor<> {
    Visitor visitor_;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename SeenCycles::value_type Cycle;
    BOOST_CONCEPT_ASSERT((boost::FrontInsertionSequenceConcept<Cycle>));

    typedef boost::unordered_map<Vertex, Edge> PredecessorMap;
    PredecessorMap predecessors_;

    SeenCycles& seen_cycles;

public:
    AllCyclesWrapper(Visitor const& v, Graph const&, SeenCycles& seen)
        : visitor_(v), seen_cycles(seen)
    { }

    void tree_edge(Edge e, Graph const& g) {
        D2_IMPL_DEBUG_ALL_CYCLES(std::cout <<
            "tree edge: " << e << '\n' <<
            "set predecessor of " << target(e, g) << " to " << e << '\n');
        predecessors_[target(e, g)] =  e;
    }

    void back_edge(Edge e, Graph const& g) {
        D2_IMPL_DEBUG_ALL_CYCLES(std::cout <<
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
        Cycle cycle;
        cycle.push_front(e);
        typedef typename PredecessorMap::const_iterator PredecessorIterator;
        while (true) {
            PredecessorIterator it = predecessors_.find(source(e, g));
            if (it == boost::end(predecessors_))
                break;

            cycle.push_front(e = it->second);
        }

        // Since it is possible to have several connected components in the
        // graph, we must make sure we do not call the visitor with redundant
        // cycles that were already found in a previous search.
        if (seen_cycles.insert(cycle).second) {
            D2_IMPL_DEBUG_ALL_CYCLES(
                std::cout << "Found cycle: ";
                std::copy(boost::begin(cycle), boost::end(cycle),
                    std::ostream_iterator<Edge>(std::cout, " "));
                std::cout << '\n';
            );
            visitor_.cycle(cycle, g);
        }
    }
};

/**
 * Dumb algorithm to compute all the cycles in a graph.
 *
 * It first does a depth first search and detects the cycles in the graph.
 * Then, it starts over a depth first search at each vertex involved in a
 * cycle found during the first pass to find the cycles in every possible
 * direction.
 *
 * The visitor's `cycle` method is called with a constant reference to a
 * sequence of unspecified type and a constant reference to the graph being
 * visited.
 */
template <typename Graph, typename Visitor>
void all_cycles(Graph const& g, Visitor const& visitor) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<Graph>::vertex_iterator VertexIter;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef std::deque<Edge> Cycle;

    D2_IMPL_DEBUG_ALL_CYCLES(std::cout <<
        "starting the all_cycles algorithm\n"
    <<  "    graph has " << num_vertices(g) << " vertices\n");

    if (num_vertices(g) == 0)
        return;

    std::set<Cycle> seen_cycles;
    typedef AllCyclesWrapper<Visitor, Graph, std::set<Cycle> > Wrapper;
    Wrapper wrapper(visitor, g, seen_cycles);

    // Perform a first dfs to find vertices involved in a cycle.
    Vertex first_vertex = *vertices(g).first;
    boost::depth_first_search(g,
        boost::root_vertex(first_vertex).visitor(wrapper));

    D2_IMPL_DEBUG_ALL_CYCLES(std::cout <<
        "first dfs done\n"
    <<  "    found " << seen_cycles.size() << " cycles\n");

    // Find all vertices involved in a cycle.
    std::set<Vertex> vertices_in_a_cycle;
    BOOST_FOREACH(Cycle const& cycle, seen_cycles) {
        vertices_in_a_cycle.insert(source(cycle[0], g));
        BOOST_FOREACH(Edge const& edge, cycle)
            vertices_in_a_cycle.insert(target(edge, g));
    }

    D2_IMPL_DEBUG_ALL_CYCLES(std::cout <<
    "    involving " << vertices_in_a_cycle.size() << " different vertices\n");

    // Start over a depth-first search at every vertex involved in a cycle.
    // This allows us to find all the different cycles in the directed graph.
    // Let's say the first dfs found a->b->a;
    // the subsequent searches will find b->a->b
    vertices_in_a_cycle.erase(first_vertex); // We already visited that one
    BOOST_FOREACH(Vertex v, vertices_in_a_cycle)
        boost::depth_first_search(g, boost::root_vertex(v).visitor(wrapper));
}
#undef D2_IMPL_DEBUG_ALL_CYCLES
} // end namespace all_cycles_detail

namespace core {
    using all_cycles_detail::all_cycles;
}
} // end namespace d2

#endif // !D2_CORE_ALL_CYCLES_HPP
