/**
 * This file defines the `AnyEvent` event.
 */

#ifndef D2_EVENTS_ANY_EVENT_HPP
#define D2_EVENTS_ANY_EVENT_HPP

#include <d2/event_traits.hpp>

#include <boost/detail/sp_typeinfo.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/count_if.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/spirit/home/support/detail/hold_any.hpp>
#include <boost/throw_exception.hpp>
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

    template <typename T>
    typename boost::enable_if<is_event<T>,
    bool>::type is_a() const {
        return boost::spirit::any_cast<T>(this) != 0;
    }

    template <typename T>
    typename boost::enable_if<is_event<T>,
    T>::type as() const {
        return boost::spirit::any_cast<T>(*this);
    }

    template <typename T>
    typename boost::enable_if<is_event<T>,
    T>::type as() {
        return boost::spirit::any_cast<T>(*this);
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

namespace detail {
    template <typename Sequence, typename Pred>
    struct all_of
        : boost::mpl::not_<
            boost::mpl::count_if<
                Sequence,
                boost::mpl::not_<Pred>
            >
        >
    { };

    template <typename Variant, typename First, typename Last>
    struct try_convert {
        typedef Variant result_type;
        result_type operator()(AnyEvent const& event) const {
            typedef typename boost::mpl::deref<First>::type T;
            typedef typename boost::mpl::next<First>::type Next;
            return event.is_a<T>() ?
                    Variant(event.as<T>()) :
                    try_convert<Variant, Next, Last>()(event);
        }
    };

    template <typename Variant, typename Last>
    struct try_convert<Variant, Last, Last> {
        typedef Variant result_type;
        result_type operator()(AnyEvent const& event) const {
            boost::throw_exception(
                boost::spirit::bad_any_cast(event.type(),
                                            BOOST_SP_TYPEID(Variant)));
            return Variant(); // never reached.
        }
    };
} // end namespace detail

/**
 * Create a variant given an `AnyEvent`. It tries to cast the `AnyEvent`,
 * doing a linear search until it find the right type. If the event is not
 * convertible to any of the types in the variant, `boost::bad_any_cast` is
 * thrown.
 */
template <typename Types>
typename boost::enable_if<boost::mpl::is_sequence<Types>,
typename boost::make_variant_over<Types>::type>::type
any_to_variant_of(AnyEvent const& event) {
    BOOST_MPL_ASSERT((detail::all_of<Types, is_event<boost::mpl::_1> >));
    typedef typename boost::make_variant_over<Types>::type Variant;
    return detail::try_convert<
        Variant,
        typename boost::mpl::begin<Types>::type,
        typename boost::mpl::end<Types>::type
    >()(event);
}

template <typename T1>
typename boost::disable_if<boost::mpl::is_sequence<T1>,
typename boost::make_variant_over<boost::mpl::vector<T1> >::type>::type
any_to_variant_of(AnyEvent const& event) {
    return any_to_variant_of<boost::mpl::vector<T1> >(event);
}

template <typename T1, typename T2>
typename boost::make_variant_over<boost::mpl::vector<T1, T2> >::type
any_to_variant_of(AnyEvent const& event) {
    return any_to_variant_of<boost::mpl::vector<T1, T2> >(event);
}

template <typename T1, typename T2, typename T3>
typename boost::make_variant_over<boost::mpl::vector<T1, T2, T3> >::type
any_to_variant_of(AnyEvent const& event) {
    return any_to_variant_of<boost::mpl::vector<T1, T2, T3> >(event);
}

template <typename T1, typename T2, typename T3, typename T4>
typename boost::make_variant_over<boost::mpl::vector<T1, T2, T3, T4> >::type
any_to_variant_of(AnyEvent const& event) {
    return any_to_variant_of<boost::mpl::vector<T1, T2, T3, T4> >(event);
}

} // end namespace d2

#endif // !D2_EVENTS_ANY_EVENT_HPP
