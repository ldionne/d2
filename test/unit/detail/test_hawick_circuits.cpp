/*!
 * @file
 * This file contains unit tests for the `d2::detail::hawick_circuits` and
 * the `d2::detail::hawick_unique_circuits` algorithms.
 */

#include "../graph_cycles_test.hpp"
#include <d2/detail/hawick_circuits.hpp>


namespace d2_test {
namespace {
struct hawick_circuits_algorithm {
    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        d2::detail::hawick_circuits(g, v);
    }
};

struct hawick_unique_circuits_algorithm {
    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        d2::detail::hawick_unique_circuits(g, v);
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(
    hawick_circuits,
    graph_cycles_test,
    hawick_circuits_algorithm
);

INSTANTIATE_TYPED_TEST_CASE_P(
    hawick_unique_circuits,
    graph_cycles_test,
    hawick_unique_circuits_algorithm
);
} // end anonymous namespace
} // end namespace d2_test
