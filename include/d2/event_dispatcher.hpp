/**
 * This file defines the `EventDispatcher` class.
 */

#ifndef D2_EVENT_DISPATCHER_HPP
#define D2_EVENT_DISPATCHER_HPP

#include <d2/event_traits.hpp>

#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>


namespace d2 {

/**
 * Tag type used to signify the usage of the default alternative.
 */
struct use_default { };

/**
 * Metafunction returning whether a type is `use_default`.
 */
template <typename T>
struct is_default
    : boost::is_same<T, use_default>
{ };

/**
 * Class using a policy to dispatch events with different event scopes to
 * the right sink.
 *
 * The `Policy` must be an MPL metafunction class that, when applied on an
 * event scope, yields a type that shall be used as a container of sinks
 * associated to that scope.
 *
 * Additionally, the `Policy` must have a static function named `get_sink'
 * that can be called with the sink container associated to a scope, an event
 * and the scope in question. The function must return an object that can be
 * used as an output stream for the event in question.
 *
 * The list of event scopes supported by the `EventDispatcher` is specified
 * via 3 possible ways:
 *  - The `Policy` has a member type named `event_scopes`, which is an MPL
 *    sequence of the scopes that must be supported by the `EventDispatcher`.
 *
 *  - The `EventDispatcher` is instantiated with an MPL sequence containing
 *    the scopes that must be supported by the `EventDispatcher`.
 *
 *  - The `EventDispatcher` is not instantiated with a list of event scopes
 *    and the `Policy` does not specify a nested member type. In such case,
 *    the default list of event scopes `available_scopes` is used.
 *
 * It is an error if both the `Policy` specifies a list of event scopes and
 * the `EventDispatcher` is instantiated with a list of event scopes.
 */
template <typename Policy, typename Scopes_ = use_default>
class EventDispatcher : boost::noncopyable {
    // Metafunction returning whether a type has a member named event_scopes.
    BOOST_MPL_HAS_XXX_TRAIT_DEF(event_scopes)

    // If you have a compilation error here, you have specified a list of
    // event scopes by defining a nested member type named event_scopes
    // inside the Policy, and have also instantiated the EventDispatcher
    // with a list of event scopes different from the default one. Since
    // we do not know which one prevails, you get this error.
    BOOST_MPL_ASSERT_MSG(
        (::boost::mpl::not_<
            boost::mpl::and_<
                has_event_scopes<Policy>,
                boost::mpl::not_<is_default<Scopes_> >
            >
        >::value)
    , EVENT_SCOPES_WERE_SPECIFIED_WHEN_INSTANTIATING_THE_EVENT_DISPATCHER\
_AND_WERE_ALSO_SPECIFIED_INSIDE_THE_POLICY, (Policy, Scopes_));

    // Metafunction returning the nested member type event_scopes from a type.
    template <typename Policy_>
    struct get_event_scopes {
        typedef typename Policy_::event_scopes type;
    };

    // Metafunction returning the nested member type event_scopes from a type
    // if it has such a nested member type, and `Default` otherwise.
    template <typename Policy_, typename Default = available_scopes>
    struct scopes_from_policy
        : boost::mpl::eval_if<has_event_scopes<Policy_>,
            get_event_scopes<Policy_>,
            boost::mpl::identity<Default>
        >
    { };

    // Get the list of scopes either from the Policy, or from
    // the template parameter.
    typedef typename boost::mpl::eval_if<
                is_default<Scopes_>,
                scopes_from_policy<Policy>,
                boost::mpl::identity<Scopes_>
            >::type Scopes;


    // Determine the type of the container of sinks for each scope by applying
    // the `Policy` on each scope.
    typedef typename boost::mpl::transform<Scopes, Policy>::type SinksByScope;

    // Zip the scopes with their associated container. This is necessary
    // to create the map below.
    typedef typename boost::mpl::transform<
                Scopes,
                SinksByScope,
                boost::mpl::quote2<boost::fusion::result_of::make_pair>
            >::type Zipped;

    // Create a map from event scopes to types defined by the Policy.
    typedef typename boost::fusion::result_of::as_map<Zipped>::type SinkMap;

    SinkMap sinks_;

    // Return the type associated to a scope. This should be some kind of
    // sink container, but it could be anything really.
    template <typename Scope>
    typename boost::fusion::result_of::at_key<SinkMap, Scope>::type get() {
        return boost::fusion::at_key<Scope>(sinks_);
    }

public:
    /**
     * Dispatch an event according to its traits and the
     * `Policy` of the `EventDispatcher`.
     */
    template <typename Event>
    typename boost::enable_if<is_event<Event>,
    EventDispatcher&>::type dispatch(Event const& event) {
        typedef typename event_scope<Event>::type Scope;
        Policy::get_sink(get<Scope>(), event, Scope()) << event;
        return *this;
    }
};

} // end namespace d2

#endif // !D2_EVENT_DISPATCHER_HPP
