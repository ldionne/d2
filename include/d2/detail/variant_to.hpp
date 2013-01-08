/**
 * This file defines a function to convert variants using a visitor.
 */

#ifndef D2_DETAIL_VARIANT_TO_HPP
#define D2_DETAIL_VARIANT_TO_HPP

#include <boost/variant.hpp>


namespace d2 {
namespace detail {
template <typename T>
struct VariantConverter : boost::static_visitor<T> {
    typedef T result_type;

    template <typename Visited>
    result_type operator()(Visited const& visited) const {
        return T(visited);
    }
};

/**
 * Converts a variant type to a given type by using a visitor. This is useful
 * for converting variants to `boost::any`-like types.
 */
template <typename Destination, typename Variant>
Destination variant_to(Variant const& variant) {
    return boost::apply_visitor(
                detail::VariantConverter<Destination>(), variant);
}

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_VARIANT_TO_HPP
