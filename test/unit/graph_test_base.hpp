/*!
 * @file
 * This file contains basic utilities for unit tests involving graphs.
 */

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/operators.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <ostream>


namespace d2_test {
namespace graph_test_base_detail {
//! @internal Provides several graph typedefs to derived classes.
template <typename Graph>
struct base_graph_typedefs {
    typedef Graph graph_type;
    typedef boost::graph_traits<graph_type> graph_traits;
    typedef typename graph_traits::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits::edge_descriptor edge_descriptor;

    typedef typename boost::vertex_property<graph_type>::type vertex_property;
    typedef typename boost::edge_property<graph_type>::type edge_property;

protected:
    typedef base_graph_typedefs base_graph_typedefs_;
};

//! @internal Regroups information about an edge of a graph.
template <typename Graph>
struct edge_info
    : boost::partially_ordered<edge_info<Graph> >,
      base_graph_typedefs<Graph>
{
    typedef typename edge_info::edge_property edge_property;
    typedef typename edge_info::edge_descriptor edge_descriptor;
    typedef typename edge_info::graph_type graph_type;
    typedef typename edge_info::vertex_property vertex_property;
    typedef typename edge_info::vertex_descriptor vertex_descriptor;

    edge_info(vertex_property src, edge_property edge, vertex_property tgt)
        : source(src), target(tgt), edge(edge)
    { }

    edge_info(graph_type const& g, edge_descriptor e) {
        // Importing these names is mandatory to make ADL kick in.
        using boost::source;
        using boost::target;
        using boost::get;
        this->source = get(boost::vertex_bundle, g, source(e, g));
        this->edge = get(boost::edge_bundle, g, e);
        this->target = get(boost::vertex_bundle, g, target(e, g));
    }

    friend std::ostream&
    operator<<(std::ostream& os, edge_info const& self) {
        os << "("
           << self.source << ", " << self.edge << ", " << self.target
           << ")";
        return os;
    }

    friend bool operator==(edge_info const& a, edge_info const& b) {
        return a.source == b.source && a.edge == b.edge &&
                                       a.target == b.target;
    }

    friend bool operator<(edge_info const& a, edge_info const& b) {
        if (a.source < b.source)
            return true;
        else if (a.source == b.source) {
            if (a.edge < b.edge)
                return true;
            else if (a.edge == b.edge)
                return a.target < b.target;
        }
        return false;
    }

    vertex_property source, target;
    edge_property edge;
};
} // end namespace graph_test_base_detail

//! Class providing basic utilities for unit tests involving graphs.
template <typename Derived, typename Graph, bool print_graph_on_failure = true>
struct graph_test_base
    : testing::Test,
      graph_test_base_detail::base_graph_typedefs<Graph>
{
    typedef typename graph_test_base::edge_property edge_property;
    typedef typename graph_test_base::edge_descriptor edge_descriptor;
    typedef typename graph_test_base::graph_type graph_type;
    typedef typename graph_test_base::vertex_property vertex_property;
    typedef typename graph_test_base::vertex_descriptor vertex_descriptor;

    typedef graph_test_base_detail::edge_info<Graph> edge_info;

    edge_info add_edge(vertex_property u, edge_property e, vertex_property v) {
        if (!vertices.count(u))
            add_vertex(u);
        if (!vertices.count(v))
            add_vertex(v);

        using boost::add_edge;
        edge_descriptor edesc =
                    add_edge(vertices[u], vertices[v], e, graph()).first;
        return edge_info(graph(), edesc);
    }

    void add_vertex(vertex_property u) {
        using boost::add_vertex;
        vertices[u] = add_vertex(u, graph());
    }

    void print_graph(std::ostream& os) const {
        using boost::get;
        boost::write_graphviz(os, graph(),
           boost::make_label_writer(get(boost::vertex_bundle, graph())),
           boost::make_label_writer(get(boost::edge_bundle, graph())));
    }

    void TearDown() {
        if (print_graph_on_failure && HasFailure()) {
            std::clog << "\nGraph\n"
                           "-----\n";
            print_graph(std::clog);
        }
    }

protected:
    typedef graph_test_base graph_test_base_;
    std::map<vertex_property, vertex_descriptor> vertices;

private:
    graph_type& graph()
    { return static_cast<Derived*>(this)->graph; }

    graph_type const& graph() const
    { return static_cast<Derived const*>(this)->graph; }
};
} // end namespace d2_test
