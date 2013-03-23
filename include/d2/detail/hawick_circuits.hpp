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

//! @internal Return whether a container contains a given value.
template <typename Container, typename Value>
bool contains(Container const& cont, BOOST_FWD_REF(Value) value) {
    return std::find(cont.begin(), cont.end(), boost::forward<Value>(value))
            != cont.end();
}

/*!
 * @internal
 * Algorithm finding all the cycles starting from a given vertex.
 *
 * The search is only done in the subgraph induced by the starting vertex
 * and the vertices with an index higher than the starting vertex.
 */
template <
    typename Graph,
    typename Visitor,
    typename VertexIndexMap,
    typename GetAdjacentVertices
>
struct hawick_circuits_from {
private:
    typedef boost::graph_traits<Graph> Traits;
    typedef typename Traits::vertex_descriptor Vertex;
    typedef typename Traits::edge_descriptor Edge;
    typedef typename Traits::vertices_size_type VerticesSize;
    typedef typename boost::property_traits<
                VertexIndexMap
            >::reference VertexIndex;
    typedef typename boost::result_of<
                GetAdjacentVertices(Vertex, Graph const&)
            >::type AdjacentVertices;

    typedef boost::one_bit_color_map<VertexIndexMap> BlockedMap;
    typedef boost::one_bit_color_type BlockedColor;
    static BlockedColor const blocked_false = boost::one_bit_white;
    static BlockedColor const blocked_true = boost::one_bit_not_white;

    typedef std::vector<Vertex> Stack;
    typedef std::vector<std::vector<Vertex> > ClosedMatrix;

public:
    hawick_circuits_from(Graph const& graph, Visitor& visitor,
                         VertexIndexMap const& vim, VerticesSize n_vertices)
        : graph_(graph), visitor_(visitor), vim_(vim),
          blocked_(n_vertices, vim_), closed_(n_vertices)
        {
            stack_.reserve(n_vertices);
        }

private:
    //! Return the index of a given vertex.
    VertexIndex index_of(Vertex v) const {
        return get(vim_, v);
    }


    //! Return whether a vertex `v` is closed to a vertex `u`.
    bool is_closed_to(Vertex u, Vertex v) const {
        typedef typename ClosedMatrix::const_reference VertexList;
        VertexList closed_to_u = closed_[index_of(u)];
        return contains(closed_to_u, v);
    }

    //! Close a vertex `v` to a vertex `u`.
    void close_to(Vertex u, Vertex v) {
        BOOST_ASSERT(!is_closed_to(u, v));
        closed_[index_of(u)].push_back(v);
    }


    //! Return whether a given vertex is blocked.
    bool is_blocked(Vertex v) const {
        return get(blocked_, v) == blocked_true;
    }

    //! Block a given vertex.
    void block(Vertex v) {
        put(blocked_, v, blocked_true);
    }

    //! Unblock a given vertex.
    void unblock(Vertex u) {
        typedef typename ClosedMatrix::reference VertexList;
        typedef typename ClosedMatrix::value_type::iterator ClosedVertexIter;

        put(blocked_, u, blocked_false);
        VertexList closed_to_u = closed_[index_of(u)];

        while (!closed_to_u.empty()) {
            Vertex w = closed_to_u.back();
            closed_to_u.pop_back();
            if (is_blocked(w))
                unblock(w);
        }
        BOOST_ASSERT(closed_to_u.empty());
    }


    bool circuit(Vertex start, Vertex v) {
        bool found_circuit = false;
        stack_.push_back(v);
        block(v);

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
                visitor_.cycle(const_cast<Stack const&>(stack_), graph_);
                found_circuit = true;
            }

            // If `w` is not blocked, we continue searching further down the
            // same path for a cycle with `w` in it.
            else if (!is_blocked(w) && circuit(start, w))
                found_circuit = true;
        }

        if (found_circuit)
            unblock(v);
        else
            BOOST_FOREACH(Vertex w, adj_vertices) {
                // Like above, we skip vertices that are not in the subgraph
                // we're considering.
                if (index_of(w) < index_of(start))
                    continue;

                // If `v` is not closed to `w`, we make it so.
                if (!is_closed_to(w, v))
                    close_to(w, v);
            }

        BOOST_ASSERT(v == stack_.back());
        stack_.pop_back();
        return found_circuit;
    }

public:
    void operator()(Vertex start) {
        circuit(start, start);
    }

private:
    Graph const& graph_;
    Visitor& visitor_;
    VertexIndexMap const& vim_;

    BlockedMap blocked_;
    ClosedMatrix closed_;
    Stack stack_;
};

template <
    typename GetAdjacentVertices,
    typename Graph, typename Visitor, typename VertexIndexMap
>
void call_hawick_circuits(Graph const& graph, Visitor /* by value */ visitor,
                          VertexIndexMap const& vertex_index_map) {

    typedef boost::graph_traits<Graph> Traits;
    typedef typename Traits::vertex_descriptor Vertex;
    typedef typename Traits::vertices_size_type VerticesSize;
    typedef hawick_circuits_from<
                Graph, Visitor, VertexIndexMap, GetAdjacentVertices
            > SubAlgorithm;

    VerticesSize const n_vertices = num_vertices(graph);
    BOOST_FOREACH(Vertex start, vertices(graph)) {
        // We may NOT reuse sub_algo once it has been called.
        SubAlgorithm sub_algo(graph, visitor, vertex_index_map, n_vertices);
        sub_algo(boost::move(start));
    }
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
