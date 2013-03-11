/**
 * This file defines several utility algorithms for working with graphs.
 */

#ifndef D2_CORE_GRAPH_UTILS_HPP
#define D2_CORE_GRAPH_UTILS_HPP

#include <boost/assert.hpp>
#include <boost/graph/graph_traits.hpp>
#include <utility>


namespace d2 {
namespace graph_utils_detail {
/**
 * Algorithm transforming a range delimited by [`first`, `last`) of the form:
 * @code
 *      (vertex 1, vertex 2, ..., vertex N)
 * @endcode
 * to a range of the form:
 * @code
 *      (edge 1 2 , edge 2 3, ..., edge N-1 N)
 * @endcode
 * where `edge i j` is the edge between vertex `i` and vertex `j`.
 *
 * @warning This algorithm does not work on multigraphs.
 *
 * @pre For any two adjacent vertices `(u, v)` in the range, an edge must
 *      connect `u` and `v` in the graph.
 */
template <typename VertexIterator, typename Graph, typename OutputIterator>
void vertices_to_edges(VertexIterator first, VertexIterator last,
                       Graph const& graph, OutputIterator result) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;

    if (first == last)
        return;

    Vertex u = *first;
    for (++first; first != last; ++first) {
        Vertex v = *first;
        std::pair<Edge, bool> e = edge(u, v, graph);
        BOOST_ASSERT_MSG(e.second, "found adjacent vertices in the range "
                                   "that are not adjacent in the graph");
        *result++ = e.first;
        u = v;
    }
}
} // end namespace graph_utils_detail

namespace core {
    using graph_utils_detail::vertices_to_edges;
}
} // end namespace d2

#endif // !D2_CORE_GRAPH_UTILS_HPP
