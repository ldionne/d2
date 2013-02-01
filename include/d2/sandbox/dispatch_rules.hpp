/**
 * This file defines the `dispatch_rules` class.
 */

#ifndef D2_SANDBOX_DISPATCH_RULES_HPP
#define D2_SANDBOX_DISPATCH_RULES_HPP

#include <d2/sandbox/event_traits.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/proto/proto.hpp>
#include <boost/utility/result_of.hpp>


namespace d2 {
namespace sandbox {

struct _event : boost::proto::transform<_event> {
    template <typename Expr, typename State, typename Data>
    struct impl : boost::proto::transform_impl<Expr, State, Data> {
        typedef typename boost::mpl::if_<
                    is_event<typename impl::expr>,
                    boost::proto::_expr,
                    boost::proto::_value
                >::type unwrap_event;

        typedef typename boost::result_of<
                    unwrap_event(typename impl::expr_param,
                                 typename impl::state_param,
                                 typename impl::data_param)
                >::type result_type;

        result_type operator()(typename impl::expr_param e,
                               typename impl::state_param s,
                               typename impl::data_param d) const {
            return unwrap_event()(e, s, d);
        }
    };
};

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
class dispatch_rules
    : public or_<BOOST_PP_ENUM_PARAMS(D2_MAX_DISPATCH_RULES_ARITY, A)>
{
    typedef typename dispatch_rules::proto_grammar Base;

public:
    template <typename Event>
    void operator()(Event const& e) const {
        typedef typename boost::proto::terminal<Event>::type Terminal;
        BOOST_MPL_ASSERT((matches<Terminal, dispatch_rules>));
        Terminal term = {e};
        Base::operator()(term);
    }

    template <typename Event, typename State>
    void operator()(Event const& e, State const& s) const {
        typedef typename boost::proto::terminal<Event>::type Terminal;
        BOOST_MPL_ASSERT((matches<Terminal, dispatch_rules>));
        Terminal term = {e};
        Base::operator()(term, s);
    }

    template <typename Event, typename State, typename Data>
    void operator()(Event const& e, State const& s, Data& d) const {
        typedef typename boost::proto::terminal<Event>::type Terminal;
        BOOST_MPL_ASSERT((matches<Terminal, dispatch_rules>));
        Terminal term = {e};
        Base::operator()(term, s, d);
    }
};

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_DISPATCH_RULES_HPP
