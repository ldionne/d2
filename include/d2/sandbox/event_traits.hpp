/**
 * This file defines traits to describe events.
 */

#ifndef D2_SANDBOX_EVENT_TRAITS_HPP
#define D2_SANDBOX_EVENT_TRAITS_HPP

#include <d2/detail/evaluate_args.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/void.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/type_traits/is_same.hpp>


namespace d2 {
namespace sandbox {

template <typename Event>
struct event_scope;

namespace event_traits_detail {
    BOOST_MPL_HAS_XXX_TRAIT_DEF(unspecialized_event_scope_tag)
    template <typename T>
    struct has_specialization_of_event_scope
        : boost::mpl::not_<
            has_unspecialized_event_scope_tag<event_scope<T> >
        >
    { };

    BOOST_MPL_HAS_XXX_TRAIT_NAMED_DEF(
        has_nested_event_scope_typedef, event_scope, false)
    template <typename T>
    struct nested_event_scope_typedef {
        typedef typename T::event_scope type;
    };
} // end namespace event_traits_detail

/**
 * Unary metafunction returning the event scope of `Event`.
 *
 * An event scope describes the required level of granularity when logging
 * an event.
 */
template <typename Event>
struct event_scope {
    typedef typename boost::mpl::eval_if<
                event_traits_detail::has_nested_event_scope_typedef<Event>,
                event_traits_detail::nested_event_scope_typedef<Event>,
                boost::mpl::void_
            >::type type;

    // Internal use only.
    struct unspecialized_event_scope_tag;
};

/**
 * Unary metafunction returning whether the `event_scope` metafunction is
 * defined for `T`.
 */
template <typename T>
struct has_event_scope
    : boost::mpl::or_<
        event_traits_detail::has_nested_event_scope_typedef<T>,
        event_traits_detail::has_specialization_of_event_scope<T>
    >
{ };

/**
 * Unary metafunction returning whether a type models the concept of an event.
 */
template <typename T>
struct is_event
    : has_event_scope<T>
{ };

/**
 * Binary metafunction returning whether an event has the `Scope` event scope.
 */
template <typename T, typename Scope>
struct has_scope
    : boost::mpl::and_<
        has_event_scope<T>,
        detail::evaluate_args<
            boost::is_same<
                event_scope<T>, boost::mpl::identity<Scope>
            >
        >
    >
{ };

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

#define D2_I_DEFINE_HAS_SCOPE(scope)                                        \
    template <typename T>                                                   \
    struct BOOST_PP_CAT(has_, scope) : has_scope<T, scope> { };             \
/**/
D2_I_DEFINE_HAS_SCOPE(global_scope)
D2_I_DEFINE_HAS_SCOPE(machine_scope)
D2_I_DEFINE_HAS_SCOPE(process_scope)
D2_I_DEFINE_HAS_SCOPE(thread_scope)
#undef D2_I_DEFINE_HAS_SCOPE

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_EVENT_TRAITS_HPP
