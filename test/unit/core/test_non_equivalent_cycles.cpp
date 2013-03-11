/**
 * This file contains unit tests for the `non_equivalent_cycles` algorithm.
 */

#include <d2/core/non_equivalent_cycles.hpp>
#include "../graph_cycles_test.hpp"


namespace d2_test {
struct non_equivalent_cycles_algorithm {
    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        d2::core::non_equivalent_cycles(g, v);
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(
    non_equivalent_cycles,
    graph_cycles_test,
    non_equivalent_cycles_algorithm
);
} // end namespace d2_test
