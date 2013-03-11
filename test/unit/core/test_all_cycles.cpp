/**
 * This file contains unit tests for the `all_cycles` algorithm.
 */

#include <d2/core/all_cycles.hpp>
#include "../graph_cycles_test.hpp"


namespace d2_test {
struct all_cycles_algorithm {
    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        d2::core::all_cycles(g, v);
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(
    all_cycles,
    graph_cycles_test,
    all_cycles_algorithm
);
} // end namespace d2_test
