/**
 * This file defines the `basic_event` class.
 */

#ifndef D2_EVENTS_BASIC_HPP
#define D2_EVENTS_BASIC_HPP

#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/equal_to.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/has_key.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/operators.hpp>
#include <boost/phoenix/core/argument.hpp>
#include <boost/serialization/access.hpp>


namespace d2 {

/**
 * Basic event class providing memberwise serialization, equality comparison
 * and access.
 */
template <typename Members_ = boost::fusion::map<> >
class basic_event
    : boost::equality_comparable<
        basic_event<Members_>
    >
{
public:
    typedef typename boost::fusion::result_of::as_map<Members_>::type Members;

    template <typename Key, typename Value>
    struct augment {
        typedef basic_event<
                    typename boost::fusion::result_of::push_back<
                        Members_, boost::fusion::pair<Key, Value>
                    >::type
                > type;
    };

private:
    Members members_;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const /*version*/) {
        boost::fusion::for_each(members_, ar & boost::phoenix::arg_names::_1);
    }

public:
    friend bool operator==(basic_event const& self, basic_event const& other) {
        return self.members_ == other.members_;
    }

    template <typename Key>
    friend typename boost::fusion::result_of::at_key<Members, Key>::type
    get(Key const&, basic_event& self) {
        return boost::fusion::at_key<Key>(self.members_);
    }

    template <typename Key>
    friend typename boost::fusion::result_of::at_key<Members const, Key>::type
    get(Key const&, basic_event const& self) {
        return boost::fusion::at_key<Key>(self.members_);
    }
};

/**
 * Metafunction returning whether an event has a member associated to the
 * `Key` key.
 */
template <typename Key, typename Event>
struct has_member
    : boost::fusion::result_of::has_key<
        typename Event::Members, Key
    >
{ };

/**
 * Metafunction creating an event with some members added to it from a
 * `basic_event`.
 */
template <typename Event, typename Key, typename Value>
struct augmented_event
    : Event::template augment<Key, Value>
{ };

} // end namespace d2

#endif // !D2_EVENTS_BASIC_HPP
