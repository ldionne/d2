/**
 * This file implements the `uniquely_identifiable` class.
 */

#ifndef D2_UNIQUELY_IDENTIFIABLE_HPP
#define D2_UNIQUELY_IDENTIFIABLE_HPP

#include <d2/detail/decl.hpp>

#include <boost/concept/usage.hpp>
#include <boost/concept_archetype.hpp>
#include <boost/move/utility.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/enable_if.hpp>
#include <cstddef>


namespace d2 {
/**
 * Overload of `unique_id` for unsigned integral types. For these types,
 * `unique_id` is the identity function.
 */
template <typename T>
typename boost::enable_if<
    boost::mpl::and_<
        boost::is_integral<typename boost::remove_reference<T>::type>,
        boost::is_unsigned<typename boost::remove_reference<T>::type>
    >,
BOOST_FWD_REF(T)>::type unique_id(BOOST_FWD_REF(T) t) {
    return boost::forward<T>(t);
}



/**
 * Specification of the `UniquelyIdentifiable` concept.
 *
 * A type is `UniquelyIdentifiable` iff it is possible to obtain an unsigned
 * integral identifier that is unique for any two distinct objects. This is
 * much like being able to hash an object, but the hash has to be perfect.
 */
template <typename T>
struct UniquelyIdentifiable {
    BOOST_CONCEPT_USAGE(UniquelyIdentifiable) {
        using ::d2::unique_id;
        assert_unsigned_integral(unique_id(val));
    }

private:
    T& val;

    template <typename U>
    static void assert_unsigned_integral(U const&) {
        BOOST_MPL_ASSERT((
            boost::mpl::and_<
                boost::is_integral<U>, boost::is_unsigned<U>
            >
        ));
    }

    // Silence MSVC warning C4610: ... user defined constructor required
    UniquelyIdentifiable() /*= delete*/;

    // Silence MSVC warning C4512: assignment operator could not be generated
    UniquelyIdentifiable& operator=(UniquelyIdentifiable const&) /*= delete*/;
};



namespace uniquely_identifiable_detail {
    /**
     * Archetype modeling no concept.
     *
     * @note This is an improvement over `boost::null_archetype<>`, which has
     *       a destructor even though the archetype should not necessarily be
     *       destructible.
     */
    template <typename T = int>
    class null_archetype : public boost::null_archetype<T> {
        ~null_archetype() /*= delete*/;
    };
} // end namespace uniquely_identifiable_detail

//! Archetype for the `UniquelyIdentifiable` concept.
template <typename Base = uniquely_identifiable_detail::null_archetype<> >
class uniquely_identifiable_archetype : public Base {
    typedef unsigned int any_unsigned_integral_type;

    // Silence MSVC warning C4624: destructor could not be generated
    ~uniquely_identifiable_archetype() /*= delete*/;

public:
    friend any_unsigned_integral_type
    unique_id(uniquely_identifiable_archetype const&) {
        return static_cast<any_unsigned_integral_type>(0);
    }
};



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
