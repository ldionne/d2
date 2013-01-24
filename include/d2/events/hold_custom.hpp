/**
 * This file defines the `hold_custom` event.
 */

#ifndef D2_EVENTS_HOLD_CUSTOM_HPP
#define D2_EVENTS_HOLD_CUSTOM_HPP

#include <d2/events/basic.hpp>

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_lvalue_reference.hpp>
#include <boost/utility/enable_if.hpp>


namespace d2 {

/**
 * Class representing an event holding an arbitrary load of data.
 */
template <typename Key, typename Value, typename Event = basic_event<> >
struct hold_custom
    : augmented_event<Event, Key, Value>::type
{ };

/**
 * Macro defining a custom holder event.
 */
#define D2_DEFINE_CUSTOM_HOLDER(name, key, value, accessor)                 \
template <typename Event = ::d2::basic_event<> >                            \
struct name                                                                 \
    : ::d2::hold_custom<key, value, Event>                                  \
{ };                                                                        \
                                                                            \
template <typename Event>                                                   \
typename ::boost::enable_if<                                                \
    ::d2::has_member<key, Event>,                                           \
typename ::boost::add_lvalue_reference<value>::type>::type                  \
accessor(Event& event) {                                                    \
    return get((key)(), event);                                             \
}                                                                           \
                                                                            \
template <typename Event>                                                   \
typename ::boost::enable_if<                                                \
    ::d2::has_member<key, Event>,                                           \
typename ::boost::add_lvalue_reference<                                     \
    typename ::boost::add_const<value>::type                                \
>::type>::type accessor(Event const& event) {                               \
    return get((key)(), event);                                             \
}                                                                           \
/**/

} // end namespace d2

#endif // !D2_EVENTS_HOLD_CUSTOM_HPP
