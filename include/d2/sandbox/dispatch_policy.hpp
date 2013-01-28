/**
 * This file defines the `dispatch_policy` class.
 */

#ifndef D2_SANDBOX_DISPATCH_POLICY_HPP
#define D2_SANDBOX_DISPATCH_POLICY_HPP

#include <boost/parameter.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/proto/proto.hpp>
#include <boost/utility/result_of.hpp>


namespace d2 {
namespace sandbox {

BOOST_PROTO_DEFINE_ENV_VAR(stream_type, stream_);
BOOST_PROTO_DEFINE_ENV_VAR(event_type, event_);

namespace dispatch_policy_detail {
BOOST_PARAMETER_TEMPLATE_KEYWORD(stream_factory)
BOOST_PARAMETER_TEMPLATE_KEYWORD(synchronize_locally_with)
BOOST_PARAMETER_TEMPLATE_KEYWORD(before_writing)
BOOST_PARAMETER_TEMPLATE_KEYWORD(write_with)
BOOST_PARAMETER_TEMPLATE_KEYWORD(after_writing)

typedef boost::parameter::parameters<
            boost::parameter::required<tag::stream_factory>
        ,   boost::parameter::optional<tag::synchronize_locally_with>
        ,   boost::parameter::optional<tag::before_writing>
        ,   boost::parameter::optional<tag::write_with>
        ,   boost::parameter::optional<tag::after_writing>
        > signature;

template <typename Args>
class dispatch_policy_impl
    : public boost::proto::transform<dispatch_policy_impl<Args> >
{
    template <typename Tag, typename Default = boost::parameter::void_>
    struct arg
        : boost::parameter::value_type<Args, Tag, Default>
    { };

    struct left_shift {
        template <typename Event, typename State, typename Data>
        void operator()(Event const& event, State const&, Data& data) const {
            data[stream_] << event;
        }
    };

    struct null_scoped_lock {
        template <typename Event, typename State, typename Data>
        null_scoped_lock(Event const&, State const&, Data const&) { }
    };

    typedef boost::proto::_void nothing;

    typedef typename arg<tag::stream_factory>::type StreamFactory;
    typedef typename arg<
                tag::synchronize_locally_with, null_scoped_lock
            >::type ScopedLock;
    typedef typename arg<tag::before_writing, nothing>::type BeforeWriting;
    typedef typename arg<tag::write_with, left_shift>::type Write;
    typedef typename arg<tag::after_writing, nothing>::type AfterWriting;

public:
    template <typename Expr, typename State, typename Data>
    struct impl : boost::proto::transform_impl<Expr, State, Data> {

        typedef void result_type;
        result_type operator()(typename impl::expr_param event,
                               typename impl::state_param state,
                               typename impl::data_param data) const {

            // Make sure `data_param` is an environment.
            typedef typename boost::proto::result_of::as_env<
                        typename impl::data_param
                    >::type BaseEnv;

            // Add `event` to the environment.
            typedef boost::proto::env<
                        event_type, typename impl::expr_param, BaseEnv
                    > DataWithEvent;
            DataWithEvent with_event(event, boost::proto::as_env(data));

            // Fetch the stream from the factory.
            typedef typename boost::result_of<
                        StreamFactory(typename impl::expr_param,
                                      typename impl::state_param,
                                      DataWithEvent&)
                    >::type Stream;
            Stream stream = StreamFactory()(event, state, with_event);

            // Add `stream` to the environment.
            typedef boost::proto::env<
                        stream_type, Stream, DataWithEvent
                    > DataWithStream;
            DataWithStream with_stream(stream, with_event);

            ScopedLock lock (event, state, with_stream);
            BeforeWriting() (event, state, with_stream);
            Write()         (event, state, with_stream);
            AfterWriting()  (event, state, with_stream);
        }
    };
};
} // end namespace dispatch_policy_detail

using dispatch_policy_detail::stream_factory;
using dispatch_policy_detail::synchronize_locally_with;
using dispatch_policy_detail::before_writing;
using dispatch_policy_detail::write_with;
using dispatch_policy_detail::after_writing;

template <
    typename A0,
    BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(4, typename B,boost::parameter::void_)
>
struct dispatch_policy
    : dispatch_policy_detail::dispatch_policy_impl<
        typename dispatch_policy_detail::signature::bind<
            A0, BOOST_PP_ENUM_PARAMS(4, B)
        >::type
    >
{ };

} // end namespace sandbox
} // end namespace d2


#include <boost/mpl/bool.hpp>

namespace boost { namespace proto {
template <BOOST_PP_ENUM_PARAMS(5, typename A)>
struct is_callable<d2::sandbox::dispatch_policy<BOOST_PP_ENUM_PARAMS(5, A)> >
    : boost::mpl::true_
{ };
}}

#endif // !D2_SANDBOX_DISPATCH_POLICY_HPP
