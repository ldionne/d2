/*!
 * @file
 * This file contains a parametrizable unit test for algorithms finding cycles
 * in a directed graph.
 */

#include "graph_test_base.hpp"
#include <d2/detail/as_cycle_visitor.hpp>
#include <d2/detail/cyclic_permutation.hpp>

#include <algorithm>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/move/utility.hpp>
#include <boost/phoenix.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <vector>


namespace d2_test {
/*!
 * Unit test template for algorithms finding cycles in directed graphs.
 *
 * @tparam Algorithm A functor performing the algorithm under test. It must be
 *         possible to call the algorithm like `algorithm(graph, functor)`.
 *         The algorithm must yield sequences of vertex descriptors to the
 *         functor.
 */
template <typename Algorithm>
struct graph_cycles_test
    : graph_test_base<
        graph_cycles_test<Algorithm>, boost::directed_graph<char, std::string>
    >
{
    typedef typename graph_cycles_test::graph_type graph_type;
    typedef typename graph_cycles_test::vertex_property vertex_property;
    typedef typename graph_cycles_test::vertex_descriptor vertex_descriptor;
    typedef std::vector<vertex_property> cycle_type;

    graph_type graph;

    /*!
     * Validate the results of a test.
     *
     * It runs the algorithm on the graph that should have been built in the
     * unit test and compares the result of the algorithm with `expected`.
     * The test fails if the results don't match.
     */
    template <typename Cycles>
    void validate_found_cycles(Cycles const& expected) {
        using namespace boost::phoenix;
        using namespace boost::phoenix::arg_names;
        actual_.clear();
        expected_.clear();
        expected_.insert(expected.begin(), expected.end());

        // Run the algorithm and gather cycles of vertex descriptors.
        std::vector<std::vector<vertex_descriptor> > vdesc_cycles;
        Algorithm()(graph,
            d2::detail::on_cycle(push_back(boost::ref(vdesc_cycles), _1)));

        // Transform the cycles of vertex descriptors into cycles of
        // vertex properties.
        std::vector<std::vector<vertex_property> > vprop_cycles;
        BOOST_FOREACH(std::vector<vertex_descriptor> const& vdesc_cycle,
                                                               vdesc_cycles) {
            std::vector<vertex_property> vprop_cycle;
            BOOST_FOREACH(vertex_descriptor vdesc, vdesc_cycle)
                vprop_cycle.push_back(get(boost::vertex_bundle, graph, vdesc));
            actual_.insert(vprop_cycle);
        }

        ASSERT_TRUE(expected_ == actual_);
    }

    void TearDown() {
        if (this->HasFailure()) {
            std::clog << "Expected cycles\n"
                         "---------------\n";
            print_cycles(std::clog, expected_);

            std::clog << "\nActual cycles\n"
                           "-------------\n";
            print_cycles(std::clog, actual_);

            graph_cycles_test::graph_test_base_::TearDown();
        }
    }

private:
    template <typename Cycles>
    static void print_cycles(std::ostream& os, Cycles const& cycles) {
        typedef typename Cycles::const_reference Cycle;
        typedef typename Cycles::value_type::value_type T;
        BOOST_FOREACH(Cycle const& cycle, cycles) {
            std::copy(cycle.begin(), cycle.end(),
                std::ostream_iterator<T>(os, " "));
            os << '\n';
        }
    }

    std::multiset<cycle_type> expected_, actual_;
};

TYPED_TEST_CASE_P(graph_cycles_test);

TYPED_TEST_P(graph_cycles_test, finds_trivial_cycle_AB) {
    using namespace boost::assign;
    typedef typename graph_cycles_test<TypeParam>::cycle_type cycle_type;

    this->add_edge('A', "", 'B');
    this->add_edge('B', "", 'A');

    this->validate_found_cycles(
            list_of<cycle_type>
            (
                list_of('A')('B')
            ));
}

TYPED_TEST_P(graph_cycles_test, finds_both_cycles_ABC) {
    using namespace boost::assign;
    typedef typename graph_cycles_test<TypeParam>::cycle_type cycle_type;

    this->add_edge('A', "", 'B');
    this->add_edge('B', "", 'C');
    this->add_edge('C', "", 'A');
    this->add_edge('A', "", 'C');

    this->validate_found_cycles(
            list_of<cycle_type>
            (
                list_of('A')('B')('C')
            )(
                list_of('A')('C')
            ));
}

REGISTER_TYPED_TEST_CASE_P(
    graph_cycles_test,
        finds_trivial_cycle_AB,
        finds_both_cycles_ABC
);
} // end namespace d2_test
