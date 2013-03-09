/**
 * This file implements the `uniquely_identifiable` class.
 */

#ifndef D2_UNIQUELY_IDENTIFIABLE_HPP
#define D2_UNIQUELY_IDENTIFIABLE_HPP

#include <d2/detail/atomic.hpp>

#include <boost/assert.hpp>
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
#include <limits>


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
 * integral identifier that is unique for any two distinct objects.
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



/**
 * Mixin class making its `Derived` type a model of the `UniquelyIdentifiable`
 * concept.
 *
 * For two different instantiations of `uniquely_identifiable`, say
 * `uniquely_identifiable<T>` and `uniquely_identifiable<U>`, the
 * counters generating unique identifiers are guaranteed to be distinct.
 *
 * @see `UniquelyIdentifiable`
 *
 * @tparam Derived Type to be made a model of the `UniquelyIdentifiable`
 *                 concept.
 */
template <typename Derived>
class uniquely_identifiable {
    /**
     * @internal
     * Unsigned integral type used to hold unique identifiers.
     *
     * The unique identifiers could eventually wrap if too many are generated.
     * An assertion will prevent this from happening silently in debug mode,
     * but in release mode this could cause several objects to have the same
     * identifier.
     *
     * If you suspect this might happen with your usage pattern of the
     * library, please contact the maintainer of the project so the
     * issue can be addressed.
     */
    typedef std::size_t unique_id_type;

    unique_id_type id_;

    /**
     * @internal
     * Return a reference to the unique identifier counter. This is to avoid
     * the static initialization order fiasco.
     */
    static detail::atomic<unique_id_type>& get_counter() {
        static detail::atomic<unique_id_type> counter(0);
        return counter;
    }

public:
    //! Construct an object with a new and unique identifier.
    uniquely_identifiable()
        : id_(get_counter()++)
    {
        BOOST_ASSERT_MSG(id_ < std::numeric_limits<unique_id_type>::max(),
            "unique identifiers are about to wrap");
    }

    /**
     * Return an unsigned integral value representing the unique
     * identifier of `*this`.
     */
    friend unique_id_type unique_id(Derived const& self) {
        return static_cast<uniquely_identifiable const&>(self).id_;
    }
};
} // end namespace d2

#endif // !D2_UNIQUELY_IDENTIFIABLE_HPP
