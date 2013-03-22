/*!
 * @file
 * This file defines the `d2::detail::vertex_to_edge_path` functor wrapper.
 */

#ifndef D2_DETAIL_VERTEX_TO_EDGE_PATH_HPP
#define D2_DETAIL_VERTEX_TO_EDGE_PATH_HPP

#include <boost/assert.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/move/utility.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/tuple/tuple.hpp> // for boost::tie
#include <vector>


namespace d2 {
namespace vertex_to_edge_path_detail {
template <typename Graph, typename F, typename EdgePathMaker>
class algorithm {
    typedef boost::graph_traits<Graph> Traits;
    typedef typename Traits::vertex_descriptor VertexDescriptor;
    typedef typename Traits::edge_descriptor EdgeDescriptor;
    typedef typename Traits::out_edge_iterator OutEdgeIterator;
    typedef typename boost::mpl::apply<
                EdgePathMaker, EdgeDescriptor
            >::type EdgePath;

    EdgePath edges_;
    Graph const& graph_;
    F& f_;

public:
    template <typename VertexPathSize>
    algorithm(Graph const& graph, F& f, VertexPathSize vpath_size)
        : graph_(graph), f_(f)
    {
        // we need n-1 edges to chain n vertices
        edges_.reserve(vpath_size - 1);
    }

    template <typename Iterator>
    void operator()(Iterator first, Iterator last) {
        VertexDescriptor u = *first++;
        if (first == last) {
            // Make sure the wrapped functor does not modify the path.
            f_(const_cast<EdgePath const&>(edges_), graph_);
            return;
        }
        VertexDescriptor v = *first;

        OutEdgeIterator oe, oe_end;
        boost::tie(oe, oe_end) = out_edges(u, graph_);

        // For all out edges of u, we only consider those whose target is v.
        for (; oe != oe_end; ++oe) {
            EdgeDescriptor e = *oe;
            if (target(e, graph_) != v)
                continue;

            edges_.push_back(e);
            (*this)(first, last);
            edges_.pop_back();
        }
    }
};

struct make_vector {
    template <typename T>
    struct apply {
        typedef std::vector<T> type;
    };
};
} // end namespace vertex_to_edge_path_detail

namespace detail {
/*!
 * Functor wrapper for graph algorithms yielding vertex paths.
 *
 * The wrapper transforms a path of the form
 * @code
 *      (vertex descriptor 1, vertex descriptor 2, ..., vertex descriptor N)
 * @endcode
 * to one or more paths of the form
 * @code
 *      (edge descriptor 1 2, edge descriptor 2 3, ..., edge descriptor N-1 N)
 * @endcode
 *
 * For every possible path of the second form, the wrapped functor is called
 * with the corresponding path.
 *
 * The reason why a single path of vertices can create several paths of edges
 * is because of parallel edges in multigraphs. For non-multigraphs, it is
 * guaranteed that a single path will be yielded.
 *
 * @tparam F The type of the wrapped functor.
 * @tparam EdgePathMaker
 *         An optional `Boost.MPL` metafunction class returning a container
 *         to store a path of edge descriptors when applied to an edge
 *         descriptor type. The default is a metafunction class returning
 *         a `std::vector`.
 */
template <
    typename F,
    typename EdgePathMaker = vertex_to_edge_path_detail::make_vector
>
class vertex_to_edge_path {
    // Mutable is to avoid being affected by the constness of
    // the functor's operator().
    F mutable f_;

public:
    explicit vertex_to_edge_path(F const& f)
        : f_(f)
    { }

    template <typename VertexPath, typename Graph>
    void operator()(VertexPath const& vp, Graph const& graph) const {
        typedef typename VertexPath::size_type VertexPathSize;
        VertexPathSize const size = vp.size();

        BOOST_ASSERT_MSG(size >= 2,
            "a path of less than 2 vertices can't possibly converted "
            "to a path of edges");
        typedef vertex_to_edge_path_detail::algorithm<
                    Graph, F, EdgePathMaker
                > Algorithm;
        Algorithm(graph, f_, size)(vp.begin(), vp.end());
    }
};

template <typename F>
vertex_to_edge_path<F> make_vertex_to_edge_path(BOOST_FWD_REF(F) f) {
    return vertex_to_edge_path<F>(boost::forward<F>(f));
}
} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_VERTEX_TO_EDGE_PATH_HPP
