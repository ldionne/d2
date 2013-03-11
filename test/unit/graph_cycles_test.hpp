/**
 * This file contains a parametrizable unit test for algorithms finding cycles
 * in a directed graph.
 */

#include <d2/core/cyclic_permutation.hpp>

#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/move/utility.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <utility>
#include <vector>


namespace d2_test {
namespace graph_cycles_test_detail {
/**
 * Visitor transforming each cycle to a sequence made of the properties of the
 * vertices in the cycle. The transformed cycles are then accumulated into
 * a `CycleContainer`.
 *
 * In other words, it transforms cycles of the form
 * @code
 *      (edge_descriptor1, edge_descriptor2, ..., edge_descriptorN)
 * @endcode
 *
 * to cycles of the form
 * @code
 *      (
 *          (vertex_property1, vertex_property2),
 *          (vertex_property2, vertex_property3),
 *          ...,
 *          (vertex_propertyN-1, vertex_propertyN)
 *      )
 * @endcode
 */
template <typename CycleContainer>
class GatherCycles {
    typedef typename CycleContainer::value_type Cycle;
    typedef typename Cycle::value_type Edge;

    CycleContainer& out_;

public:
    explicit GatherCycles(CycleContainer& out)
        : out_(out)
    { }

    template <typename EdgeDescriptorCycle, typename Graph>
    void cycle(EdgeDescriptorCycle const& cycle, Graph const& graph) const {
        typedef boost::graph_traits<Graph> Traits;
        typedef typename Traits::edge_descriptor EdgeDescriptor;
        typedef typename Traits::vertex_descriptor VertexDescriptor;

        typedef typename boost::property_map<
                    Graph, boost::vertex_bundle_t
                >::const_type VertexPropertyMap;

        typedef typename boost::property_traits<
                    VertexPropertyMap
                >::value_type VertexProperty;

        Cycle prop_cycle;
        BOOST_FOREACH(EdgeDescriptor e, cycle) {
            VertexDescriptor source = boost::source(e, graph),
                             target = boost::target(e, graph);
            VertexProperty src = boost::get(boost::vertex_bundle, graph, source),
                           tgt = boost::get(boost::vertex_bundle, graph, target);

            prop_cycle.insert(prop_cycle.end(), Edge(src, tgt));
        }
        out_.insert(out_.end(), boost::move(prop_cycle));
    }
};
} // end namespace graph_cycles_test_detail

/**
 * Unit test template for algorithms finding cycles in directed graphs.
 *
 * The `TestTraits` template parameters must customize the test using
 * the following elements:
 *  - A nested type named `algorithm` representing a functor that can be
 *    called like `f(graph, visitor)`. The algorithm must accept a visitor
 *    with the interface of `GatherCycles` above.
 */
template <typename TestTraits>
struct graph_cycles_test : testing::Test {
    typedef typename TestTraits::algorithm Algorithm;
    typedef boost::directed_graph<char> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
    typedef boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;

    typedef std::pair<char, char> Edge;
    typedef std::vector<Edge> Cycle;
    typedef std::vector<Cycle> CycleContainer;

    Graph graph;

    /**
     * Runs the algorithm on the graph that should have been built in the
     * unit test and compares the result of the algorithm with `expected_`.
     *
     * If the results don't match, the test fails.
     */
    void validate_found_cycles(CycleContainer const& expected_) {
        typedef graph_cycles_test_detail::GatherCycles<CycleContainer> Visitor;
        // Gather the actual cycles in the graph that should have been
        // constructed in the test.
        Algorithm()(graph, Visitor(actual));

        expected = expected_;

        ASSERT_TRUE(d2::core::is_cyclic_permutation(expected, actual));
    }

    void TearDown() {
        if (HasFailure()) {
            std::clog << "Expected cycles\n"
                         "---------------\n";
            print_cycles(std::clog, expected);

            std::clog << "\nActual cycles\n"
                           "-------------\n";
            print_cycles(std::clog, actual);

            std::clog << "\nGraph\n"
                           "-----\n";
            boost::write_graphviz(std::clog, graph, boost::make_label_writer(
                                    boost::get(boost::vertex_bundle, graph)));
        }
    }

private:
    CycleContainer expected, actual;

    static void print_cycles(std::ostream& os, CycleContainer const& cycles) {
        BOOST_FOREACH(Cycle const& cycle, cycles) {
            BOOST_FOREACH(Edge const& edge, cycle)
                os << "(" << edge.first << ", " << edge.second << ") ";
            os << '\n';
        }
    }
};

TYPED_TEST_CASE_P(graph_cycles_test);

/**
 * @internal
 * Because of Gtest's implementation of template unit tests, which uses
 * inheritance, we need to import the names of the base class in order
 * to use them. This macro makes it easy.
 *
 * However, note that we still need to use `this->` to access the `graph`
 * and `validate_found_cycles()` members of `graph_cycles_test`.
 */
#define D2_I_IMPORT_TEMPLATE_UNIT_TEST_STUFF()                              \
    typedef graph_cycles_test<TypeParam> GraphCycleTest;                    \
    typedef typename GraphCycleTest::VertexDescriptor VertexDescriptor;     \
    typedef typename GraphCycleTest::EdgeDescriptor EdgeDescriptor;         \
    typedef typename GraphCycleTest::CycleContainer CycleContainer;         \
    typedef typename GraphCycleTest::Cycle Cycle;                           \
    typedef typename GraphCycleTest::Edge Edge;                             \
    typedef typename GraphCycleTest::Graph Graph;                           \
/**/

TYPED_TEST_P(graph_cycles_test, finds_trivial_cycle_AB) {
    D2_I_IMPORT_TEMPLATE_UNIT_TEST_STUFF()
    using namespace boost::assign;

    VertexDescriptor A = add_vertex('A', this->graph);
    VertexDescriptor B = add_vertex('B', this->graph);
    add_edge(A, B, this->graph);
    add_edge(B, A, this->graph);

    Cycle AB = list_of(Edge('A', 'B'));

    this->validate_found_cycles(list_of(AB));
}

TYPED_TEST_P(graph_cycles_test, finds_both_cycles_ABC) {
    D2_I_IMPORT_TEMPLATE_UNIT_TEST_STUFF()
    using namespace boost::assign;

    VertexDescriptor A = add_vertex('A', this->graph);
    VertexDescriptor B = add_vertex('B', this->graph);
    VertexDescriptor C = add_vertex('C', this->graph);
    add_edge(A, B, this->graph);
    add_edge(B, C, this->graph);
    add_edge(C, A, this->graph);
    add_edge(A, C, this->graph);

    Cycle ABC = list_of(Edge('A', 'B'))(Edge('B', 'C'))(Edge('C', 'A'));
    Cycle AC = list_of(Edge('A', 'C'))(Edge('C', 'A'));

    this->validate_found_cycles(list_of(ABC)(AC));
}

#undef D2_I_IMPORT_TEMPLATE_UNIT_TEST_STUFF

REGISTER_TYPED_TEST_CASE_P(
    graph_cycles_test,
        finds_trivial_cycle_AB,
        finds_both_cycles_ABC
);
} // end namespace d2_test
