/*!
 * @file
 * This file contains unit tests for the `boost::tiernan_all_cycles` algorithm.
 */

#include "../graph_cycles_test.hpp"
#include <d2/detail/tiernan_all_cycles.hpp>


namespace d2_test {
struct tiernan_all_cycles_algorithm {
    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        boost::tiernan_all_cycles(g, v);
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(
    tiernan_all_cycles,
    graph_cycles_test,
    tiernan_all_cycles_algorithm
);
} // end namespace d2_test
