/*!
 * @file
 * This file implements the `d2::detail::hawick_circuits` algorithm.
 */

#ifndef D2_DETAIL_HAWICK_CIRCUITS_HPP
#define D2_DETAIL_HAWICK_CIRCUITS_HPP

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/properties.hpp>
#include <boost/move/utility.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/result_of.hpp>
#include <set>
#include <utility> // for std::pair
#include <vector>


namespace d2 {
namespace hawick_circuits_detail {
//! @internal Functor returning all the vertices adjacent to a vertex.
struct get_all_adjacent_vertices {
    template <typename Sig>
    struct result;

    template <typename This, typename Vertex, typename Graph>
    struct result<This(Vertex, Graph)> {
    private:
        typedef typename boost::remove_reference<Graph>::type RawGraph;
        typedef boost::graph_traits<RawGraph> Traits;
        typedef typename Traits::adjacency_iterator AdjacencyIterator;

    public:
        typedef std::pair<AdjacencyIterator, AdjacencyIterator> type;
    };

    template <typename Vertex, typename Graph>
    typename result<
        get_all_adjacent_vertices(BOOST_FWD_REF(Vertex), BOOST_FWD_REF(Graph))
    >::type
    operator()(BOOST_FWD_REF(Vertex) v, BOOST_FWD_REF(Graph) g) const {
        return adjacent_vertices(boost::forward<Vertex>(v),
                                 boost::forward<Graph>(g));
    }
};

//! @internal Functor returning a set of the vertices adjacent to a vertex.
struct get_unique_adjacent_vertices {
    template <typename Sig>
    struct result;

    template <typename This, typename Vertex, typename Graph>
    struct result<This(Vertex, Graph)> {
        typedef std::set<typename boost::remove_reference<Vertex>::type> type;
    };

    template <typename Vertex, typename Graph>
    typename result<get_unique_adjacent_vertices(Vertex, Graph const&)>::type
    operator()(Vertex v, Graph const& g) const {
        typedef typename result<
                    get_unique_adjacent_vertices(Vertex, Graph const&)
                >::type Set;
        return Set(adjacent_vertices(v, g).first,
                   adjacent_vertices(v, g).second);
    }
};

/*!
 * @internal
 * Implementation of Hawick's algorithm to find circuits.
 */
template <
    typename Graph,
    typename Visitor,
    typename VertexIndexMap,
    typename GetAdjacentVertices
>
struct hawick_circuits_algorithm {
    typedef boost::graph_traits<Graph> Traits;
    typedef typename Traits::vertex_descriptor Vertex;
    typedef typename Traits::edge_descriptor Edge;
    typedef typename Traits::adjacency_iterator AdjacencyIterator;
    typedef typename Traits::vertices_size_type VerticesSize;
    typedef typename boost::property_traits<
                VertexIndexMap
            >::reference VertexIndex;

    typedef boost::one_bit_color_map<VertexIndexMap> BlockedMap;
    typedef boost::one_bit_color_type BlockedColor;
    static BlockedColor const blocked_false = boost::one_bit_white;
    static BlockedColor const blocked_true = boost::one_bit_not_white;

    typedef std::vector<Vertex> Stack;
    typedef std::vector<std::vector<Vertex> > ClosedMatrix;

    Graph const& graph_;
    // We use mutable to avoid being affected by the constness
    // of the visitor's `cycle` method.
    Visitor mutable visitor_;
    VertexIndexMap const vim_;

    hawick_circuits_algorithm(Graph const& graph,
                              Visitor const& visitor,
                              VertexIndexMap const& vertex_index_map)
        : graph_(graph), visitor_(visitor), vim_(vertex_index_map)
    { }

    VertexIndex index_of(Vertex u) const {
        return get(vim_, u);
    }

    //! Return whether a container contains a given value.
    template <typename Container, typename Value>
    static bool contains(Container const& cont, BOOST_FWD_REF(Value) value) {
        return std::find(cont.begin(), cont.end(),
                                boost::forward<Value>(value)) != cont.end();
    }

    //! Return whether a vertex `v` is closed to a vertex `u`.
    bool is_closed_to(Vertex u, Vertex v, ClosedMatrix const& closed) const {
        typedef typename ClosedMatrix::const_reference VertexList;
        VertexList list_of_closed_to_u = closed[index_of(u)];
        return contains(list_of_closed_to_u, v);
    }

    // Close a vertex `v` to a vertex `u`.
    void make_closed_to(Vertex u, Vertex v, ClosedMatrix& closed) const {
        closed[index_of(u)].push_back(v);
    }

    void unblock(Vertex u, BlockedMap& blocked, ClosedMatrix& closed) const {
        typedef typename ClosedMatrix::reference VertexList;
        typedef typename ClosedMatrix::value_type::iterator ClosedVertexIter;

        put(blocked, u, blocked_false);
        VertexList closed_to_u = closed[index_of(u)];
        for (ClosedVertexIter w_it = closed_to_u.begin();
                                        w_it != closed_to_u.end(); ++w_it) {
            Vertex w = *w_it;
            closed_to_u.erase(
                std::remove(w_it, closed_to_u.end(), w), closed_to_u.end());

            if (get(blocked, w))
                unblock(w, blocked, closed);
        }
    }

    static void block(Vertex u, BlockedMap& blocked) {
        put(blocked, u, blocked_true);
    }

    bool circuit(Vertex start, Vertex v, BlockedMap& blocked,
                 ClosedMatrix& closed, Stack& stack) const {
        bool found_circuit = false;
        stack.push_back(v);
        block(v, blocked);

        typedef typename boost::result_of<
                    GetAdjacentVertices(Vertex, Graph const&)
                >::type AdjacentVertices;
        AdjacentVertices const adj_vertices = GetAdjacentVertices()(v, graph_);

        BOOST_FOREACH(Vertex w, adj_vertices) {
            // Since we're only looking in the subgraph induced by `start` and
            // the vertices with an index higher than `start`, we skip any
            // vertex that does not satisfy that.
            if (index_of(w) < index_of(start))
                continue;

            // If the last vertex is equal to `start`, we have a circuit.
            else if (w == start) {
                // const_cast to ensure the visitor does not modify the stack
                visitor_.cycle(const_cast<Stack const&>(stack), graph_);
                found_circuit = true;
            }

            // If `w` is not blocked, we continue searching further down the
            // same path for a cycle with `w` in it.
            else if (!get(blocked, w) &&
                     circuit(start, w, blocked, closed, stack))
                found_circuit = true;
        }

        if (found_circuit)
            unblock(v, blocked, closed);
        else
            BOOST_FOREACH(Vertex w, adj_vertices) {
                // Like above, we skip vertices that are not in the subgraph
                // we're considering.
                if (index_of(w) < index_of(start))
                    continue;

                // If `v` is not closed to `w`, we make it so.
                if (!is_closed_to(w, v, closed))
                    make_closed_to(w, v, closed);
            }

        BOOST_ASSERT(v == stack.back());
        stack.pop_back();
        return found_circuit;
    }

    void operator()() {
        VerticesSize const n_vertices = num_vertices(graph_);
        BOOST_FOREACH(Vertex start, vertices(graph_)) {
            BlockedMap blocked(n_vertices, vim_);
            ClosedMatrix closed(n_vertices);
            Stack stack;
            stack.reserve(n_vertices);

            // Find the circuits in the subgraph induced by `start` and the
            // vertices with an index higher than `start`.
            circuit(start, start, blocked, closed, stack);
        }
    }
};

template <
    typename GetAdjacentVertices,
    typename Graph, typename Visitor, typename VertexIndexMap
>
void call_hawick_circuits(BOOST_FWD_REF(Graph) graph,
                          BOOST_FWD_REF(Visitor) visitor,
                          BOOST_FWD_REF(VertexIndexMap) vertex_index_map) {
    typedef typename boost::remove_reference<Graph>::type RawGraph;
    typedef typename boost::remove_reference<Visitor>::type RawVisitor;
    typedef typename boost::remove_reference<VertexIndexMap>::type RawVIM;
    typedef hawick_circuits_detail::hawick_circuits_algorithm<
                RawGraph, RawVisitor, RawVIM, GetAdjacentVertices
            > Algorithm;

    Algorithm(boost::forward<Graph>(graph),
              boost::forward<Visitor>(visitor),
              boost::forward<VertexIndexMap>(vertex_index_map))();
}

template <typename GetAdjacentVertices, typename Graph, typename Visitor>
void call_hawick_circuits(BOOST_FWD_REF(Graph) graph,
                          BOOST_FWD_REF(Visitor) visitor) {
    typedef typename boost::remove_reference<Graph>::type RawGraph;
    typedef typename boost::property_map<
                RawGraph, boost::vertex_index_t
            >::const_type VertexIndexMap;

    VertexIndexMap vim = get(boost::vertex_index, graph);
    call_hawick_circuits<GetAdjacentVertices>(boost::forward<Graph>(graph),
                                              boost::forward<Visitor>(visitor),
                                              boost::move(vim));
}
} // end namespace hawick_circuits_detail

namespace detail {
template <typename Graph, typename Visitor, typename VertexIndexMap>
void hawick_circuits(BOOST_FWD_REF(Graph) graph,
                     BOOST_FWD_REF(Visitor) visitor,
                     BOOST_FWD_REF(VertexIndexMap) vertex_index_map) {
    hawick_circuits_detail::call_hawick_circuits<
        hawick_circuits_detail::get_all_adjacent_vertices
    >(
        boost::forward<Graph>(graph),
        boost::forward<Visitor>(visitor),
        boost::forward<VertexIndexMap>(vertex_index_map)
    );
}

template <typename Graph, typename Visitor>
void hawick_circuits(BOOST_FWD_REF(Graph) graph,
                     BOOST_FWD_REF(Visitor) visitor) {
    hawick_circuits_detail::call_hawick_circuits<
        hawick_circuits_detail::get_all_adjacent_vertices
    >(boost::forward<Graph>(graph), boost::forward<Visitor>(visitor));
}

template <typename Graph, typename Visitor, typename VertexIndexMap>
void hawick_unique_circuits(BOOST_FWD_REF(Graph) graph,
                            BOOST_FWD_REF(Visitor) visitor,
                            BOOST_FWD_REF(VertexIndexMap) vertex_index_map) {
    hawick_circuits_detail::call_hawick_circuits<
        hawick_circuits_detail::get_unique_adjacent_vertices
    >(
        boost::forward<Graph>(graph),
        boost::forward<Visitor>(visitor),
        boost::forward<VertexIndexMap>(vertex_index_map)
    );
}

template <typename Graph, typename Visitor>
void hawick_unique_circuits(BOOST_FWD_REF(Graph) graph,
                            BOOST_FWD_REF(Visitor) visitor) {
    hawick_circuits_detail::call_hawick_circuits<
        hawick_circuits_detail::get_unique_adjacent_vertices
    >(boost::forward<Graph>(graph), boost::forward<Visitor>(visitor));
}
} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_HAWICK_CIRCUITS_HPP
