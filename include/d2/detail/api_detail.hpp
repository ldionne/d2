/**
 * This file contains implementation details for the API.
 *
 * @note Since this is part of the public API, we must be very careful not
 *       to depend on boost.
 */

#ifndef D2_DETAIL_API_DETAIL_HPP
#define D2_DETAIL_API_DETAIL_HPP

#include <cstddef>


namespace d2 {
namespace api_detail {
    /**
     * A poor man's boolean metatype. Remember, we can't depend on boost!
     */
    template <bool b>
    struct bool_ {
        typedef bool_ type;
        static bool const value = b;
    };

    /**
     * Metafunction returning whether two types are the same.
     */
    template <typename T, typename U> struct is_same : bool_<false> { };
    template <typename T> struct is_same<T, T> : bool_<true> { };

    /**
     * Metafunction returning whether we provide a default implementation for
     * the `unique_id` function.
     *
     * We provide a default implementation for the following types:
     *  - `unsigned {char,short,int,long}`
     *  - `std::size_t`
     *
     * @internal If `std::size_t` is the same as any other of the 4 types we
     *           explicitly specialize the template, we must not explicitly
     *           specialize the template for `std::size_t`. This is why we
     *           use the `is_same` trick instead of having one more
     *           specialization for `std::size_t`.
     */
    template <typename T> struct default_unique_id
        : bool_< ::d2::api_detail::is_same<T, std::size_t>::value>
    { };
    template <> struct default_unique_id<unsigned char> : bool_<true> { };
    template <> struct default_unique_id<unsigned short> : bool_<true> { };
    template <> struct default_unique_id<unsigned int> : bool_<true> { };
    template <> struct default_unique_id<unsigned long> : bool_<true> { };

    /**
     * Dispatch to the right implementation of `unique_id_impl`, depending
     * on whether we provide a default implementation for `unique_id`.
     */
    template <typename T>
    std::size_t unique_id_impl(T const& t) {
        return unique_id_impl(t, typename default_unique_id<T>::type());
    }

    /**
     * Call the `unique_id` function on a type for which we don't provide a
     * default implementation.
     */
    template <typename T>
    std::size_t unique_id_impl(T const& t, bool_<false>) {
        // If you get a compilation error here, you called one of the API
        // functions with a type for which a default `unique_id` is not
        // provided, and that does not provide a `unique_id` function
        // returning a type convertible to `std::size_t` that can be found
        // via ADL.
        return unique_id(t);
    }

    /**
     * The default `unique_id` implementation we provide is simply a
     * conversion to `std::size_t`.
     */
    template <typename T>
    std::size_t unique_id_impl(T const& t, bool_<true>) {
        return t;
    }

} // end namespace api_detail
} // end namespace d2

#endif // !D2_DETAIL_API_DETAIL_HPP
