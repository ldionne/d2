/**
 * This file contains a parametrizable unit test for algorithms finding cycles
 * in a directed graph.
 */

#include <d2/detail/cyclic_permutation.hpp>

#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/move/utility.hpp>
#include <boost/operators.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


namespace d2_test {
namespace graph_cycles_test_detail {
typedef char VertexProperty;
typedef std::string EdgeProperty;
typedef boost::directed_graph<VertexProperty, EdgeProperty> Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor VertexDescriptor;
typedef boost::graph_traits<Graph>::edge_descriptor EdgeDescriptor;

struct CycleElement : boost::equality_comparable<CycleElement> {
    CycleElement(VertexProperty src, EdgeProperty edge, VertexProperty tgt)
        : source(src), edge(edge), target(tgt)
    { }

    VertexProperty source;
    EdgeProperty edge;
    VertexProperty target;

    friend bool operator==(CycleElement const& a, CycleElement const& b) {
        return a.source == b.source && a.edge == b.edge && a.target == b.target;
    }
};
typedef std::vector<CycleElement> Cycle;

/**
 * Visitor transforming a cycle of the form:
 * @code
 *      (edge_descriptor 1 2, edge_descriptor 2 3, ..., edge_descriptor N-1 N)
 * @endcode
 * to a cycle of the form:
 * @code
 *      (
 *          (vertex_property 1, edge_property 1 2, vertex_property 2),
 *          (vertex_property 2, edge_property 2 3, vertex_property 3),
 *          ...,
 *          (vertex_property N-1, edge_property N-1 N, vertex_property N)
 *      )
 * @endcode
 * where each 3-tuple `(vertex_property, edge_property, vertex_property)` is
 * a `CycleElement`.
 *
 * Also, the cycles are accumulated into an output iterator.
 */
template <typename OutputIterator>
class gather_cycles_type {
    OutputIterator out_;

public:
    explicit gather_cycles_type(OutputIterator const& out)
        : out_(out)
    { }

    template <typename EdgeDescriptorCycle>
    void operator()(EdgeDescriptorCycle const& c, Graph const& g) {
        cycle(c, g);
    }

    template <typename EdgeDescriptorCycle>
    void cycle(EdgeDescriptorCycle const& cycle, Graph const& graph) {
        Cycle transformed;
        BOOST_FOREACH(EdgeDescriptor e, cycle) {
            VertexDescriptor src = boost::source(e, graph),
                             tgt = boost::target(e, graph);

            CycleElement element(
                boost::get(boost::vertex_bundle, graph, src),
                boost::get(boost::edge_bundle, graph, e),
                boost::get(boost::vertex_bundle, graph, tgt)
            );
            transformed.push_back(boost::move(element));
        }
        *out_++ = boost::move(transformed);
    }
};

template <typename OutputIterator>
gather_cycles_type<OutputIterator> gather_cycles(OutputIterator const& out) {
    return gather_cycles_type<OutputIterator>(out);
}
} // end namespace graph_cycles_test_detail

/**
 * Unit test template for algorithms finding cycles in directed graphs.
 *
 * @tparam Algorithm A functor performing the algorithm under test. It must be
 *         possible to call the algorithm like `algorithm(graph, visitor)`,
 *         where visitor has the same interface as that of
 *         `gather_cycles_type`. The algorithm must yield sequences of
 *         edge descriptors to the visitor.
 */
template <typename Algorithm>
struct graph_cycles_test : testing::Test {
private:
    typedef graph_cycles_test_detail::Graph Graph;
    typedef graph_cycles_test_detail::Cycle Cycle;

    std::vector<Cycle> expected_, actual_;

public:
    Graph graph;

    /**
     * Runs the algorithm on the graph that should have been built in the
     * unit test and compares the result of the algorithm with `expected`.
     *
     * If the results don't match, the test fails.
     */
    template <typename Cycles>
    void validate_found_cycles(Cycles const& expected) {
        using namespace graph_cycles_test_detail;
        actual_.clear();
        expected_.clear();

        // Gather the actual cycles in the graph that should have been
        // constructed in the test.
        Algorithm()(graph, gather_cycles(std::back_inserter(actual_)));
        expected_.assign(expected.begin(), expected.end());

        ASSERT_TRUE(d2::detail::is_cyclic_permutation(expected_, actual_));
    }

    void TearDown() {
        if (HasFailure()) {
            std::clog << "Expected cycles\n"
                         "---------------\n";
            print_cycles(std::clog, expected_);

            std::clog << "\nActual cycles\n"
                           "-------------\n";
            print_cycles(std::clog, actual_);

            std::clog << "\nGraph\n"
                           "-----\n";
            boost::write_graphviz(std::clog, graph, boost::make_label_writer(
                                    boost::get(boost::vertex_bundle, graph)));
        }
    }

private:
    template <typename Cycles>
    static void print_cycles(std::ostream& os, Cycles const& cycles) {
        using namespace graph_cycles_test_detail;
        BOOST_FOREACH(Cycle const& cycle, cycles) {
            BOOST_FOREACH(CycleElement const& elt, cycle)
                os << "(" << elt.source << ", "
                          << elt.edge << ", "
                          << elt.target << ") ";
            os << '\n';
        }
    }
};

TYPED_TEST_CASE_P(graph_cycles_test);

TYPED_TEST_P(graph_cycles_test, finds_trivial_cycle_AB) {
    using namespace graph_cycles_test_detail;
    using namespace boost::assign;

    VertexDescriptor A = add_vertex('A', this->graph);
    VertexDescriptor B = add_vertex('B', this->graph);
    add_edge(A, B, this->graph);
    add_edge(B, A, this->graph);

    Cycle AB = list_of(CycleElement('A', "", 'B'))
                      (CycleElement('B', "", 'A'));

    this->validate_found_cycles(list_of(AB));
}

TYPED_TEST_P(graph_cycles_test, finds_both_cycles_ABC) {
    using namespace graph_cycles_test_detail;
    using namespace boost::assign;

    VertexDescriptor A = add_vertex('A', this->graph);
    VertexDescriptor B = add_vertex('B', this->graph);
    VertexDescriptor C = add_vertex('C', this->graph);
    add_edge(A, B, this->graph);
    add_edge(B, C, this->graph);
    add_edge(C, A, this->graph);
    add_edge(A, C, this->graph);

    Cycle ABC = list_of(CycleElement('A', "", 'B'))
                       (CycleElement('B', "", 'C'))
                       (CycleElement('C', "", 'A'));

    Cycle AC = list_of(CycleElement('A', "", 'C'))
                      (CycleElement('C', "", 'A'));

    this->validate_found_cycles(list_of(ABC)(AC));
}

TYPED_TEST_P(graph_cycles_test, finds_multiple_cycles_in_multigraph_AB) {
    using namespace graph_cycles_test_detail;
    using namespace boost::assign;

    VertexDescriptor A = add_vertex('A', this->graph);
    VertexDescriptor B = add_vertex('B', this->graph);
    add_edge(A, B, "AB1", this->graph);
    add_edge(B, A, "BA1", this->graph);

    add_edge(A, B, "AB2", this->graph);
    add_edge(B, A, "BA2", this->graph);

    Cycle AB1_BA1 = list_of(CycleElement('A', "AB1", 'B'))
                           (CycleElement('B', "BA1", 'A'));

    Cycle AB1_BA2 = list_of(CycleElement('A', "AB1", 'B'))
                           (CycleElement('B', "BA2", 'A'));

    Cycle AB2_BA1 = list_of(CycleElement('A', "AB2", 'B'))
                           (CycleElement('B', "BA1", 'A'));

    Cycle AB2_BA2 = list_of(CycleElement('A', "AB2", 'B'))
                           (CycleElement('B', "BA2", 'A'));

    this->validate_found_cycles(list_of(AB1_BA1)(AB1_BA2)(AB2_BA1)(AB2_BA2));
}

REGISTER_TYPED_TEST_CASE_P(
    graph_cycles_test,
        finds_trivial_cycle_AB,
        finds_both_cycles_ABC,
        finds_multiple_cycles_in_multigraph_AB
);
} // end namespace d2_test
