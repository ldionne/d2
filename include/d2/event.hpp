/**
 * This file defines the `Event` class.
 */

#ifndef D2_EVENT_HPP
#define D2_EVENT_HPP

#include <boost/functional/hash.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/operators.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/serialization/access.hpp>
#include <cstddef>


namespace d2 {

namespace event_detail {
template <typename Event>
struct EventIndex {
    static std::size_t const value;
};

/**
 * Return the unique identifier associated to the `Event` type.
 *
 * @warning Using this function requires that the `Event` type has been
 *          registered using the `D2_REGISTER_EVENT` macro.
 */
template <typename Event>
std::size_t event_type_index() {
    return ::d2::event_detail::EventIndex<Event>::value;
}

/**
 * Metafunction returning whether a type is registered.
 */
template <typename Event>
struct is_registered
    : boost::mpl::false_
{ };

/**
 * Macro to associate a type to a unique identifier.
 *
 * @note This macro must be used at global scope.
 * @todo Use a perfect hash function. The current implementation will break if
 *       there are collisions between hashes.
 */
#define D2_REGISTER_EVENT(type)                                             \
    namespace d2 { namespace event_detail {                                 \
        template <>                                                         \
        ::std::size_t const ::d2::event_detail::EventIndex<type>::value     \
                          = ::boost::hash_value(BOOST_PP_STRINGIZE(type));  \
        template <>                                                         \
        struct ::d2::event_detail::is_registered<type>                      \
            : ::boost::mpl::true_                                           \
        { };                                                                \
    }}                                                                      \
/**/
} // end namespace event_detail

/**
 * Mixin used to generate event types.
 */
template <typename Derived, typename Members_>
class Event : boost::equality_comparable<Derived> {
    typedef typename boost::fusion::result_of::as_map<Members_>::type Members;

    Members members_;

    Derived& derived() {
        return *static_cast<Derived*>(this);
    }

    Derived const& derived() const {
        return *static_cast<Derived const*>(this);
    }

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const version) {
        derived().do_serialize(ar, version);
    }

protected:
    /**
     * Provide serialization to the derived class. We make this method
     * protected so it can be extended in the derived class.
     */
    template <typename Archive>
    void do_serialize(Archive& ar, unsigned int const) {
        ar & members_;
    }

    //! Convenience typedef for derived classes.
    typedef Event Event_;

    //! Convenience method for derived classes.
    Event& base_event() { return *this; }
    Event const& base_event() const { return *this; }

public:
    friend bool operator==(Event const& self, Event const& other) {
        return self.members_ == other.members_;
    }

    template <typename Key>
    friend typename boost::fusion::result_of::at_key<Members, Key>::type
    get(Key const&, Derived& self) {
        return boost::fusion::at_key<Key>(self.members_);
    }

    template <typename Key>
    friend typename boost::fusion::result_of::at_key<Members const, Key>::type
    get(Key const&, Derived const& self) {
        return boost::fusion::at_key<Key>(self.members_);
    }
};

} // end namespace d2

#endif // !D2_EVENT_HPP
