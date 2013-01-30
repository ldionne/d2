/**
 * This file defines the `event` class.
 */

#ifndef D2_SANDBOX_EVENT_HPP
#define D2_SANDBOX_EVENT_HPP

#include <boost/mpl/remove.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/parameter.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/proto/proto.hpp>


namespace d2 {
namespace sandbox {

namespace event_detail {
BOOST_PARAMETER_TEMPLATE_KEYWORD(scope)
namespace tag { struct members; }

#define D2_MAX_EVENT_MEMBERS 10

namespace members_detail { struct not_used; }

template <
    BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
        D2_MAX_EVENT_MEMBERS, typename A, members_detail::not_used
    )
>
struct members
    : boost::parameter::template_keyword<
        tag::members,
        boost::mpl::remove<
            boost::mpl::vector<BOOST_PP_ENUM_PARAMS(D2_MAX_EVENT_MEMBERS, A)>,
            members_detail::not_used
        >
    >
{ };

typedef boost::parameter::parameters<
            boost::parameter::required<tag::scope>
        ,   boost::parameter::optional<tag::members>
        > signature;

template <typename Args>
class event_impl {
    template <typename Tag, typename Default = boost::parameter::void_>
    struct arg
        : boost::parameter::value_type<Args, Tag, Default>
    { };

    typedef typename arg<tag::members, members<> >::type Members;
    typedef typename arg<tag::scope>::type Scope;

public:
};
} // end namespace event_detail

using event_detail::members;
using event_detail::scope;

template <
    typename Arg0, typename Arg1 = boost::parameter::void_,
    typename EnableADL = boost::proto::is_proto_expr
>
class event {
    typedef typename event_detail::signature::bind<Arg0, Arg1>::type Args;

public:
    BOOST_PROTO_EXTENDS(
        typename boost::proto::terminal<event_detail::event_impl<Args> >::type,
        event,
        boost::proto::default_domain
    )
};

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_EVENT_HPP
