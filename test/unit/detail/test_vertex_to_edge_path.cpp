/*!
 * @file
 * This file contains unit tests for the `d2::detail::vertex_to_edge_path`
 * functor wrapper.
 */

#include "../graph_test_base.hpp"
#include <d2/detail/cyclic_permutation.hpp>
#include <d2/detail/vertex_to_edge_path.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/phoenix.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <ostream>
#include <set>
#include <string>
#include <vector>


namespace d2_test {
namespace {
struct vertex_to_edge_path_test
    : graph_test_base<
        vertex_to_edge_path_test, boost::directed_graph<char, std::string>
    >
{
    typedef std::vector<edge_info> EdgePath;
    graph_type graph;

    void TearDown() {
        if (HasFailure()) {
            std::clog << "Expected paths\n"
                         "--------------\n";
            print_edge_paths(std::clog, expected_);
            std::clog << '\n';

            std::clog << "\nActual paths\n"
                           "------------\n";
            print_edge_paths(std::clog, actual_);
            std::clog << '\n';

            graph_test_base_::TearDown();
        }
    }

    void validate_paths(std::vector<vertex_property> const& vprop_path,
                        std::vector<EdgePath> const& expected_einfo_paths) {
        expected_.clear();
        expected_.insert(expected_einfo_paths.begin(),
                         expected_einfo_paths.end());
        actual_.clear();

        // First, map the vertex properties back to their vertex descriptor.
        std::vector<vertex_descriptor> vdesc_path;
        BOOST_FOREACH(vertex_property vprop, vprop_path) {
            BOOST_ASSERT(vertices.count(vprop));
            vdesc_path.push_back(vertices[vprop]);
        }

        // Then, compute the edge paths with `vertex_to_edge_path`.
        std::vector<std::vector<edge_descriptor> > edesc_paths;
        using namespace boost::phoenix;
        d2::detail::make_vertex_to_edge_path(
            push_back(boost::ref(edesc_paths), arg_names::_1)
        )(vdesc_path, graph);

        // Finally, transform edge descriptors to `edge_info`s.
        BOOST_FOREACH(std::vector<edge_descriptor> const& edesc_path,
                                                                edesc_paths) {
            std::vector<edge_info> einfo_path;
            BOOST_FOREACH(edge_descriptor edesc, edesc_path)
                einfo_path.push_back(edge_info(graph, edesc));
            actual_.insert(einfo_path);
        }

        ASSERT_TRUE(expected_ == actual_);
    }

private:
    template <typename EdgePaths>
    void print_edge_paths(std::ostream& os, EdgePaths const& eps) {
        BOOST_FOREACH(EdgePath const& ep, eps)
            std::copy(ep.begin(), ep.end(),
                std::ostream_iterator<edge_info>(os, " "));
    }

    std::multiset<EdgePath> actual_, expected_;
};

using namespace boost::assign;

TEST_F(vertex_to_edge_path_test, trivial_between_two_vertices) {
    add_edge('A', "A1B", 'B');
    add_edge('A', "A2B", 'B');

    validate_paths(
        list_of('A')('B'),
            list_of<EdgePath>
            (
                list_of(edge_info('A', "A1B", 'B'))
            )(
                list_of(edge_info('A', "A2B", 'B'))
            ));
}

TEST_F(vertex_to_edge_path_test, no_multi_edges_yields_single_path) {
    add_edge('A', "AB", 'B');
    add_edge('B', "BC", 'C');

    validate_paths(
        list_of('A')('B')('C'),
            list_of<EdgePath>
            (
                list_of(edge_info('A', "AB", 'B'))
                       (edge_info('B', "BC", 'C'))
            ));
}

TEST_F(vertex_to_edge_path_test, self_loop_works_as_expected) {
    add_edge('A', "1", 'A');
    add_edge('A', "2", 'A');

    validate_paths(
        list_of('A')('A'),
            list_of<EdgePath>
            (
                list_of(edge_info('A', "1", 'A'))
            )(
                list_of(edge_info('A', "2", 'A'))
            ));
}
} // end anonymous namespace
} // end namespace d2_test
