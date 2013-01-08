/**
 * This file defines the `Repository` class.
 */

#ifndef D2_REPOSITORY_HPP
#define D2_REPOSITORY_HPP

#include <d2/sandbox/container_view.hpp>

#include <boost/fusion/include/all.hpp>
#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/unordered_map.hpp>
#include <fstream>


namespace d2 {

struct boost_unordered_maps_only {
    template <typename Key, typename Value>
    struct apply {
        typedef boost::unordered_map<Key, Value> type;
    };
};

template <typename Keys,
          typename Mapping = boost_unordered_maps_only>
class Repository {
    // A policy for deciding input only, output only
    // or input/output would be nice.
    typedef std::ifstream Istream;
    typedef std::ofstream Ostream;
    typedef std::fstream IOStream;

    // Associate a Key to its map type using the Mapping policy and the
    // default stream type.
    struct AssociateToMap {
        template <typename Key>
        struct apply {
            typedef typename boost::mpl::apply<Mapping, Key, IOStream>::type
                                                                        Value;
            typedef boost::fusion::pair<Key, Value> type;
        };
    };

    // Zip the Keys with their associated container. This is necessary
    // to create the map below.
    typedef typename boost::mpl::transform<Keys, AssociateToMap>::type Zipped;

    // Create a map from Keys to types determined by our Mapping policy.
    // This fusion map acts like our instance variables.
    typedef typename boost::fusion::result_of::as_map<Zipped>::type Dict;

    Dict streams_;

    // Return the map (instance variable) associated to a Key.
    template <typename Key>
    struct map_at {
        typedef typename boost::remove_reference<
            typename boost::fusion::result_of::at_key<Dict, Key>::type
        >::type type;

        type& operator()(Dict& dict) const {
            return boost::fusion::at_key<Key>(dict);
        }

        typedef typename boost::remove_reference<
            typename boost::fusion::result_of::at_key<Dict const, Key>::type
        >::type const_type;

        const_type& operator()(Dict const& dict) const {
            return boost::fusion::at_key<Key>(dict);
        }
    };

    template <typename Key, template <typename Container> class View>
    struct make_view {
        typedef typename View<typename map_at<Key>::type>::type type;
        typedef typename View<typename map_at<Key>::const_type>::type
                                                                const_type;
    };

public:
    template <typename Source>
    explicit Repository(Source const& root) {

    }

    template <typename Key>
    struct key_view
        : make_view<Key, sandbox::key_view>
    { };

    template <typename Key>
    struct value_view
        : make_view<Key, sandbox::value_view>
    { };

    template <typename Key>
    struct item_view
        : make_view<Key, sandbox::item_view>
    { };

    template <typename Key>
    typename item_view<Key>::type items() {
        typedef typename item_view<Key>::type View;
        return View(map_at<Key>()(streams_));
    }

    template <typename Key>
    typename item_view<Key>::const_type items() const {
        typedef typename item_view<Key>::const_type View;
        return View(map_at<Key>()(streams_));
    }

    template <typename Key>
    typename value_view<Key>::type values() {
        typedef typename value_view<Key>::type View;
        return View(map_at<Key>()(streams_));
    }

    template <typename Key>
    typename value_view<Key>::const_type values() const {
        typedef typename value_view<Key>::const_type View;
        return View(map_at<Key>()(streams_));
    }

    template <typename Key>
    typename key_view<Key>::type keys() {
        typedef typename key_view<Key>::type View;
        return View(map_at<Key>()(streams_));
    }

    template <typename Key>
    typename key_view<Key>::const_type keys() const {
        typedef typename key_view<Key>::const_type View;
        return View(map_at<Key>()(streams_));
    }

    template <typename Key>
    typename map_at<Key>::type::mapped_type& operator[](Key const& key) {
        return map_at<Key>()(streams_)[key];
    }

private:
    struct map_is_empty {
        typedef bool result_type;

        template <typename Map>
        result_type operator()(Map const& map) const {
            return map.second.empty();
        }
    };

public:
    bool empty() const {
        return boost::fusion::all(streams_, map_is_empty());
    }
};

} // end namespace d2

#endif // !D2_REPOSITORY_HPP
