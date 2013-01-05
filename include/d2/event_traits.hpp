/**
 * This file defines traits describing the events captured by the
 * logging system.
 */

#ifndef D2_EVENT_TRAITS_HPP
#define D2_EVENT_TRAITS_HPP

#include <boost/exception/detail/is_output_streamable.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/back.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/type_traits/is_convertible.hpp>


namespace d2 {

/**
 * Unary metafunction returning the event scope of `Event`.
 *
 * An event scope describes the level of granularity required when logging
 * an event.
 */
template <typename Event>
struct event_scope {
    typedef typename Event::event_scope type;
};

/**
 * Events within this scope are logged globally.
 */
struct global_scope { };

/**
 * Events within this scope are logged on a per-machine basis.
 */
struct machine_scope { };

/**
 * Events within this scope are logged on a per-process basis.
 */
struct process_scope { };

/**
 * Events within this scope are logged on a per-thread basis.
 */
struct thread_scope { };

/**
 * MPL sequence containing all the scopes that are currently available, from
 * the coarsest scope to the finest scope. This can be useful for
 * metaprogramming purposes.
 */
typedef boost::mpl::vector<
            global_scope,
            machine_scope,
            process_scope,
            thread_scope
        > available_scopes;

/**
 * Events within this scope must be logged at the least granular level
 * available (towards `global_scope`).
 *
 * @see `finest_scope`
 */
typedef boost::mpl::front<available_scopes>::type coarsest_scope;

/**
 * Events within this scope must be logged at the most granular level
 * available (towards `thread_scope`).
 *
 * @see `coarsest_scope`
 */
typedef boost::mpl::back<available_scopes>::type finest_scope;


/**
 * Unary metafunction returning the ordering policy of `Event`.
 *
 * An ordering policy describes how an event should be logged relatively
 * to other events __in the same event scope__.
 */
template <typename Event>
struct ordering_policy {
    typedef typename Event::ordering_policy type;
};

/**
 * Events with this ordering policy may be logged in any order.
 */
struct no_order_policy { };

/**
 * Events with this ordering policy must be logged in the same order
 * as they were generated.
 */
struct strict_order_policy { };


namespace detail {
BOOST_MPL_HAS_XXX_TRAIT_DEF(event_scope)
BOOST_MPL_HAS_XXX_TRAIT_DEF(ordering_policy)

template <typename T>
struct is_output_streamable
    : boost::mpl::bool_<
        ::boost::is_output_streamable<T>::value
    >
{ };

/**
 * Adapts a metafunction by evaluating its parameters before forwarding
 * to the original metafunction.
 */
template <typename F>
struct evaluate_args;

template <template <typename> class F, typename A1>
struct evaluate_args<F<A1> > {
    typedef typename F<typename A1::type>::type type;
};

template <template <typename, typename> class F, typename A1, typename A2>
struct evaluate_args<F<A1, A2> > {
    typedef typename F<typename A1::type, typename A2::type>::type type;
};
} // end namespace detail

/**
 * Unary metafunction returning whether a type is an event type.
 */
template <typename T>
struct is_event
    : boost::mpl::and_<
        detail::has_event_scope<T>,
        detail::has_ordering_policy<T>,
        detail::is_output_streamable<T>
    >
{ };

/**
 * Binary metafunction returning whether a type has the `Scope` event scope.
 */
template <typename T, typename Scope>
struct has_event_scope
    : boost::mpl::and_<
        detail::has_event_scope<T>,
        detail::evaluate_args<
            boost::is_convertible<
                event_scope<T>, boost::mpl::identity<Scope>
            >
        >
    >
{ };

/**
 * Binary metafunction returning whether a type has the `Policy`
 * ordering policy.
 */
template <typename T, typename Policy>
struct has_ordering_policy
    : boost::mpl::and_<
        detail::has_ordering_policy<T>,
        detail::evaluate_args<
            boost::is_convertible<
                ordering_policy<T>, boost::mpl::identity<Policy>
            >
        >
    >
{ };

} // end namespace d2

#endif // !D2_EVENT_TRAITS_HPP
