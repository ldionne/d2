/*!
 * @file
 * This file contains unit tests for the `d2::detail::hawick_circuits`
 * algorithm.
 */

#include "../graph_cycles_test.hpp"
#include <d2/detail/as_cycle_visitor.hpp>
#include <d2/detail/hawick_circuits.hpp>
#include <d2/detail/vertex_to_edge_path.hpp>


namespace d2_test {
namespace {
struct hawick_circuits_algorithm {
    template <typename F>
    struct on_call {
        explicit on_call(F const& f) : f_(f) { }

        template <typename Path, typename Graph>
        void operator()(Path const& p, Graph const& g) {
            f_.cycle(p, g);
        }

        F f_;
    };

    template <typename Graph, typename Visitor>
    void operator()(Graph const& g, Visitor const& v) const {
        typedef on_call<Visitor> Functor;
        typedef d2::detail::vertex_to_edge_path<Functor> VertexToEdgePath;
        d2::detail::hawick_circuits(g,
            d2::detail::on_cycle(VertexToEdgePath(Functor(v))));
    }
};

INSTANTIATE_TYPED_TEST_CASE_P(
    hawick_circuits,
    graph_cycles_test,
    hawick_circuits_algorithm
);
} // end anonymous namespace
} // end namespace d2_test
