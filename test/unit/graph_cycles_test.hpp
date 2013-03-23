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
#include <string>
#include <vector>


namespace d2_test {
/*!
 * Unit test template for algorithms finding cycles in directed graphs.
 *
 * @tparam Algorithm A functor performing the algorithm under test. It must be
 *         possible to call the algorithm like `algorithm(graph, functor)`.
 *         The algorithm must yield sequences of edge descriptors to the
 *         functor.
 */
template <typename Algorithm>
struct graph_cycles_test
    : graph_test_base<
        graph_cycles_test<Algorithm>, boost::directed_graph<char, std::string>
    >
{
    typedef typename graph_cycles_test::graph_type graph_type;
    typedef typename graph_cycles_test::edge_info edge_info;
    typedef typename graph_cycles_test::edge_descriptor edge_descriptor;
    typedef std::vector<edge_info> cycle_type;

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

        // Run the algorithm and gather cycles of edge descriptors.
        std::vector<std::vector<edge_descriptor> > edesc_cycles;
        Algorithm()(graph,
            d2::detail::on_cycle(push_back(boost::ref(edesc_cycles), _1)));

        // Transform the cycles of edge descriptors into cycles of edge_info.
        std::vector<std::vector<edge_info> > einfo_cycles;
        BOOST_FOREACH(std::vector<edge_descriptor> const& edesc_cycle,
                                                               edesc_cycles) {
            std::vector<edge_info> einfo_cycle;
            BOOST_FOREACH(edge_descriptor edesc, edesc_cycle)
                einfo_cycle.push_back(edge_info(graph, edesc));
            actual_.push_back(einfo_cycle);
        }

        expected_.assign(expected.begin(), expected.end());

        ASSERT_TRUE(d2::detail::is_cyclic_permutation(expected_, actual_));
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
        BOOST_FOREACH(Cycle const& cycle, cycles) {
            std::copy(cycle.begin(), cycle.end(),
                std::ostream_iterator<edge_info>(os, " "));
            os << '\n';
        }
    }

    std::vector<cycle_type> expected_, actual_;
};

TYPED_TEST_CASE_P(graph_cycles_test);

TYPED_TEST_P(graph_cycles_test, finds_trivial_cycle_AB) {
    using namespace boost::assign;
    typedef typename graph_cycles_test<TypeParam>::edge_info edge_info;
    typedef typename graph_cycles_test<TypeParam>::cycle_type cycle_type;

    this->add_edge('A', "", 'B');
    this->add_edge('B', "", 'A');

    cycle_type AB = list_of(edge_info('A', "", 'B'))
                           (edge_info('B', "", 'A'));

    this->validate_found_cycles(list_of(AB));
}

TYPED_TEST_P(graph_cycles_test, finds_both_cycles_ABC) {
    using namespace boost::assign;
    typedef typename graph_cycles_test<TypeParam>::edge_info edge_info;
    typedef typename graph_cycles_test<TypeParam>::cycle_type cycle_type;

    this->add_edge('A', "", 'B');
    this->add_edge('B', "", 'C');
    this->add_edge('C', "", 'A');
    this->add_edge('A', "", 'C');

    cycle_type ABC = list_of(edge_info('A', "", 'B'))
                            (edge_info('B', "", 'C'))
                            (edge_info('C', "", 'A'));

    cycle_type AC = list_of(edge_info('A', "", 'C'))
                           (edge_info('C', "", 'A'));

    this->validate_found_cycles(list_of(ABC)(AC));
}

TYPED_TEST_P(graph_cycles_test, finds_multiple_cycles_in_multigraph_AB) {
    using namespace boost::assign;
    typedef typename graph_cycles_test<TypeParam>::edge_info edge_info;
    typedef typename graph_cycles_test<TypeParam>::cycle_type cycle_type;

    this->add_edge('A', "AB1", 'B');
    this->add_edge('B', "BA1", 'A');

    this->add_edge('A', "AB2", 'B');
    this->add_edge('B', "BA2", 'A');

    cycle_type AB1_BA1 = list_of(edge_info('A', "AB1", 'B'))
                                (edge_info('B', "BA1", 'A'));

    cycle_type AB1_BA2 = list_of(edge_info('A', "AB1", 'B'))
                                (edge_info('B', "BA2", 'A'));

    cycle_type AB2_BA1 = list_of(edge_info('A', "AB2", 'B'))
                                (edge_info('B', "BA1", 'A'));

    cycle_type AB2_BA2 = list_of(edge_info('A', "AB2", 'B'))
                                (edge_info('B', "BA2", 'A'));

    this->validate_found_cycles(list_of(AB1_BA1)(AB1_BA2)(AB2_BA1)(AB2_BA2));
}

REGISTER_TYPED_TEST_CASE_P(
    graph_cycles_test,
        finds_trivial_cycle_AB,
        finds_both_cycles_ABC,
        finds_multiple_cycles_in_multigraph_AB
);
} // end namespace d2_test
