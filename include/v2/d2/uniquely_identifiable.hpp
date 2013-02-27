/**
 * This file implements the `uniquely_identifiable` class.
 */

#ifndef D2_UNIQUELY_IDENTIFIABLE_HPP
#define D2_UNIQUELY_IDENTIFIABLE_HPP

#include <d2/detail/config.hpp>

#include <cstddef>


namespace d2 {
namespace uniquely_identifiable_detail {
    /**
     * Unsigned integral type used to hold unique identifiers.
     *
     * @note This type is not part of the public API.
     */
    typedef std::size_t unsigned_integral_type;

    /**
     * Return a new unsigned integral value each time it is called.
     * This function can be considered atomic.
     *
     * @note The unique identifiers could wrap if this function is called too
     *       many times. An assertion will prevent this from happening
     *       silently in debug mode, but in release mode this could cause
     *       several synchronization objects to have the same identifier.
     *       In such case, the analysis could yield undefined results.
     *
     *       If you suspect this might happen with your usage pattern of the
     *       library, please contact the maintainer of the project so the
     *       issue can be addressed.
     *
     * @note This function is not part of the public API.
     */
    D2_DECL extern unsigned_integral_type get_unique_id();
} // end namespace uniquely_identifiable_detail

/**
 * Mixin class making its `Derived` class a model of the
 * `UniquelyIdentifiable` concept.
 *
 * @see `UniquelyIdentifiable`
 *
 * @note The identifiers are unique across the instances of this template
 *       (with any template parameter). For example, instances of
 *       `uniquely_identifiable<T>` and instances of `uniquely_identifiable<U>`
 *       are guaranteed to have different identifiers just like two instances
 *       of `uniquely_identifiable<T>` are guaranteed to.
 *
 * @note Using this class requires linking with the `d2` library.
 */
template <typename Derived>
class uniquely_identifiable {
    typedef uniquely_identifiable_detail::unsigned_integral_type
                                                    unsigned_integral_type;
    unsigned_integral_type id_;

public:
    //! Construct an object with a new and unique identifier.
    uniquely_identifiable()
        : id_(uniquely_identifiable_detail::get_unique_id())
    { }

    /**
     * Return an unsigned integral value representing the unique
     * identifier of `*this`.
     */
    friend unsigned_integral_type unique_id(Derived const& self) {
        return static_cast<uniquely_identifiable const&>(self).id_;
    }
};
} // end namespace d2

#endif // !D2_UNIQUELY_IDENTIFIABLE_HPP
