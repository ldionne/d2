/**
 * This file contains unit tests for the `multidigraph_all_cycles` algorithm.
 */

#include "../graph_cycles_test.hpp"
#include <d2/detail/multidigraph_all_cycles.hpp>


namespace d2_test {
struct multidigraph_all_cycles_algorithm {
    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        d2::detail::multidigraph_all_cycles(g, v);
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(
    multidigraph_all_cycles,
    graph_cycles_test,
    multidigraph_all_cycles_algorithm
);
} // end namespace d2_test
