/**
 * This file defines the `AnyEvent` event.
 */

#ifndef D2_EVENTS_ANY_EVENT_HPP
#define D2_EVENTS_ANY_EVENT_HPP

#include <d2/event_traits.hpp>

#include <boost/spirit/home/support/detail/hold_any.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/variant.hpp>


namespace d2 {

/**
 * Event capable of holding any type for which the `is_event` metafunction
 * is true.
 * @note Technically, this is not an event because it's missing some
 *       member typedefs. Therefore, the `is_event` metafunction will
 *       return false for `AnyEvent`.
 */
class AnyEvent : public boost::spirit::hold_any {
    // hold_any is much faster than boost::any and it is streamable,
    // so it is exactly what we need.
    typedef boost::spirit::hold_any Base;

public:
    AnyEvent() { }

    template <typename T>
    AnyEvent(T const& t, typename boost::enable_if<is_event<T> >::type* =0)
        : Base(t)
    { }

    AnyEvent(AnyEvent const& other)
        : Base(other)
    { }

    template <typename T>
    typename boost::enable_if<is_event<T>,
    AnyEvent&>::type operator=(T const& t) {
        Base::operator=(t);
        return *this;
    }

    AnyEvent& operator=(AnyEvent const& other) {
        Base::operator=(other);
        return *this;
    }
};


namespace detail {
template <typename T>
struct ConvertVariantTo : boost::static_visitor<T> {
    template <typename Visited>
    T operator()(Visited const& visited) const {
        return T(visited);
    }
};
} // end namespace detail

/**
 * Convert a variant type to a given type. This is useful for converting
 * variants to `boost::any`-like types.
 */
template <typename Destination, typename Variant>
Destination variant_to(Variant const& variant) {
    return boost::apply_visitor(
                detail::ConvertVariantTo<Destination>(), variant);
}

} // end namespace d2

#endif // !D2_EVENTS_ANY_EVENT_HPP
