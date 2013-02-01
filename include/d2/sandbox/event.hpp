/**
 * This file defines the `event` class.
 */

#ifndef D2_SANDBOX_EVENT_HPP
#define D2_SANDBOX_EVENT_HPP

#include <d2/detail/pair_adjacent.hpp>
#include <d2/sandbox/parameter.hpp>

#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/remove.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/operators.hpp>
#include <boost/phoenix/core/argument.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/proto/proto.hpp>
#include <boost/serialization/access.hpp>


namespace d2 {
namespace sandbox {

namespace event_detail {
D2_PARAMETER_TEMPLATE_KEYWORD(scope)
namespace tag { struct members; }

struct unused;

template <typename Members>
struct sanitize_members
    : detail::pair_adjacent<
        typename boost::mpl::remove<Members, unused>::type,
        boost::fusion::pair<boost::mpl::_1, boost::mpl::_2>
    >
{ };

#define D2_MAX_EVENT_MEMBERS 10
template <BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
                                    D2_MAX_EVENT_MEMBERS, typename A, unused)>
struct members
    : parameter_template_keyword<
        tag::members,
        typename sanitize_members<
            boost::mpl::vector<BOOST_PP_ENUM_PARAMS(D2_MAX_EVENT_MEMBERS, A)>
        >::type
    >
{ };

template <typename Members_, typename Scope>
class auto_event : boost::equality_comparable<auto_event<Members_, Scope> > {
    typedef typename boost::fusion::result_of::as_map<Members_>::type Members;
    Members members_;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const /*version*/) {
        boost::fusion::for_each(members_, ar & boost::phoenix::arg_names::_1);
    }

public:
    typedef Scope event_scope;

    friend bool operator==(auto_event const& self, auto_event const& other) {
        return self.members_ == other.members_;
    }

    template <typename Key>
    friend typename boost::fusion::result_of::at_key<Members, Key>::type
    get(Key const&, auto_event& self) {
        return boost::fusion::at_key<Key>(self.members_);
    }

    template <typename Key>
    friend typename boost::fusion::result_of::at_key<Members const, Key>::type
    get(Key const&, auto_event const& self) {
        return boost::fusion::at_key<Key>(self.members_);
    }
};

typedef boost::parameter::parameters<
            boost::parameter::required<tag::scope>,
            boost::parameter::optional<tag::members>
        > make_auto_event_signature;

template <typename Args>
class make_auto_event {
    template <typename Tag, typename Default = boost::parameter::void_>
    struct arg
        : parameter_value_type<Args, Tag, Default>
    { };

    typedef typename members<>::value_type NoMembers;
    typedef typename arg<tag::members, NoMembers>::type Members;

    typedef typename arg<tag::scope>::type Scope;

public:
    typedef auto_event<Members, Scope> type;
};
} // end namespace event_detail

using event_detail::scope;
using event_detail::members;

template <typename A0, typename A1 = boost::parameter::void_>
struct event
    : event_detail::make_auto_event<
        typename event_detail::make_auto_event_signature::bind<A0, A1>::type
    >::type
{ };

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_EVENT_HPP
