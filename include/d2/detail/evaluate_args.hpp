/**
 * This file defines the `evaluate_args` metafunction.
 */

#ifndef D2_DETAIL_EVALUATE_ARGS_HPP
#define D2_DETAIL_EVALUATE_ARGS_HPP

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>


namespace d2 {
namespace detail {

/**
 * Macro controlling the maximum arity of metafunctions handled by
 * `evaluate_args`.
 */
#define D2_EVALUATE_ARGS_MAX_ARITY 10

/**
 * Adapts a metafunction by evaluating its parameters before forwarding
 * to the original metafunction.
 */
template <typename F>
struct evaluate_args {
    typedef typename F::type type;
};

#define D2_I_EVALUATE_METAFUNCTION(unused, N, Arg) \
    typename BOOST_PP_CAT(Arg, N)::type

#define D2_I_EVALUATE_ARGS_SPEC(Z, N, unused)                               \
template <                                                                  \
    template <BOOST_PP_ENUM_PARAMS_Z(Z, N, typename A)> class F,            \
    BOOST_PP_ENUM_PARAMS_Z(Z, N, typename A)                                \
>                                                                           \
struct evaluate_args<F<BOOST_PP_ENUM_PARAMS_Z(Z, N, A)> > {                 \
    typedef typename F<                                                     \
                BOOST_PP_ENUM_ ## Z(N, D2_I_EVALUATE_METAFUNCTION, A)       \
            >::type type;                                                   \
};                                                                          \
/**/

BOOST_PP_REPEAT_FROM_TO(
    1, D2_EVALUATE_ARGS_MAX_ARITY, D2_I_EVALUATE_ARGS_SPEC, ~
)

#undef D2_I_EVALUATE_METAFUNCTION
#undef D2_I_EVALUATE_ARGS_SPEC

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_EVALUATE_ARGS_HPP
