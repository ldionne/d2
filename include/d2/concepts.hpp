/**
 * This file defines concepts used within `d2`.
 */

#ifndef D2_CONCEPTS_HPP
#define D2_CONCEPTS_HPP

#include <boost/concept/usage.hpp>
#include <boost/concept_archetype.hpp>
#include <boost/mpl/and.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/utility/enable_if.hpp>
#include <cstddef>


namespace d2 {

/**
 * Partial specialization of `unique_id` for unsigned integral types. For
 * these types, `unique_id` is the identity function.
 */
template <typename T>
typename boost::enable_if<
            boost::mpl::and_<boost::is_integral<T>, boost::is_unsigned<T> >,
std::size_t>::type unique_id(T const& t) {
    return static_cast<std::size_t>(t);
}

/**
 * Concept specification of the `UniquelyIdentifiable` concept.
 *
 * A type is `UniquelyIdentifiable` iff it is possible to obtain an unsigned
 * integral identifier that is unique for any two distinct objects. This is
 * much like being able to hash an object, but the hash has to be perfect.
 */
template <typename T>
struct UniquelyIdentifiable {
    BOOST_CONCEPT_USAGE(UniquelyIdentifiable) {
        using ::d2::unique_id;
        std::size_t id = unique_id(val);
        (void)id;
    }

private:
    T const& val;
    // Silence MSVC warning C4610: ... user defined constructor required
    UniquelyIdentifiable() /*= delete*/;
    // Silence MSVC warning C4512: assignment operator could not be generated
    UniquelyIdentifiable& operator=(UniquelyIdentifiable const&) /*= delete*/;
};

namespace detail {
    /**
     * Archetype modelling no concept.
     * @note This is an improvement over `boost::null_archetype<>`, which has
     *       a destructor even though the archetype should not necessarily be
     *       destructible.
     */
    template <typename T = int>
    class null_archetype : public boost::null_archetype<T> {
        ~null_archetype() /*= delete*/;
    };
} // end namespace detail

/**
 * Archetype for the `UniquelyIdentifiable` concept.
 */
template <typename Base = detail::null_archetype<> >
struct uniquely_identifiable_archetype : Base {
    friend std::size_t unique_id(uniquely_identifiable_archetype const&) {
        return 0;
    }

private:
    // Silence MSVC warning C4624: destructor could not be generated
    ~uniquely_identifiable_archetype() /*= delete*/;
};

} // end namespace d2

#endif // !D2_CONCEPTS_HPP
