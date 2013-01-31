/**
 * This file defines the `dispatch_rules` class.
 */

#ifndef D2_SANDBOX_DISPATCH_RULES_HPP
#define D2_SANDBOX_DISPATCH_RULES_HPP

#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/proto/matches.hpp>
#include <boost/proto/proto_fwd.hpp>


namespace d2 {
namespace sandbox {

typedef boost::proto::_expr _event;
using boost::proto::_;
using boost::proto::_data;
using boost::proto::_state;
using boost::proto::and_;
using boost::proto::if_;
using boost::proto::matches;
using boost::proto::not_;
using boost::proto::or_;
using boost::proto::when;

#define D2_MAX_DISPATCH_RULES_ARITY BOOST_PROTO_MAX_LOGICAL_ARITY

template <
    BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
        D2_MAX_DISPATCH_RULES_ARITY, typename A, not_<_>
    )
>
struct dispatch_rules
    : or_<
        BOOST_PP_ENUM_PARAMS(D2_MAX_DISPATCH_RULES_ARITY, A)
    >
{ };

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_DISPATCH_RULES_HPP
