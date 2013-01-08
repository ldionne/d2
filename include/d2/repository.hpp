/**
 * This file defines the `Repository` class.
 */

#ifndef D2_REPOSITORY_HPP
#define D2_REPOSITORY_HPP

#include <d2/detail/exceptions.hpp>
#include <d2/sandbox/container_view.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/fusion/include/all.hpp>
#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/unordered_map.hpp>
#include <fstream>
#include <ios>
#include <string>
#include <typeinfo>


namespace d2 {

/**
 * Base class for exceptions related to the `Repository` class.
 */
struct RepositoryException : virtual Exception {
    virtual char const* what() const throw() {
        return "d2::RepositoryException";
    }
};

/**
 * Exception thrown when a `Repository` is created with an invalid path.
 */
struct InvalidRepositoryPathException : virtual RepositoryException {
    virtual char const* what() const throw() {
        return "d2::InvalidRepositoryPathException";
    }
};

/**
 * Exception thrown when a `Repository` is unable to open a new stream.
 */
struct StreamApertureException : virtual RepositoryException {
    virtual char const* what() const throw() {
        return "d2::StreamApertureException";
    }
};

namespace exception_tag {
    struct target_filename;
}
typedef boost::error_info<exception_tag::target_filename, char const*>
                                                            TargetFilename;

/**
 * Default mapping policy using `boost::unordered_map`s to map key
 * instances to streams.
 */
struct boost_unordered_maps_only {
    template <typename Key, typename Value>
    struct apply {
        typedef boost::unordered_map<Key, Value> type;
    };
};

/**
 * Default naming policy using the typeid and `boost::lexical_cast` to
 * derive a name for a stream associated to a key.
 */
struct use_typeid_and_boost_lexical_cast {
    typedef std::string result_type;

    template <typename T>
    result_type operator()(T const& t) const {
        return typeid(T).name() + boost::lexical_cast<std::string>(t);
    }
};

/**
 * Class representing a repository into which stuff can be stored.
 */
template <typename Keys,
          typename NamingPolicy = use_typeid_and_boost_lexical_cast,
          typename MappingPolicy = boost_unordered_maps_only>
class Repository {
    // A policy for deciding input only, output only
    // or input/output would be nice.
    typedef std::ifstream Istream;
    typedef std::ofstream Ostream;
    typedef std::fstream IOStream;

    // Associate a Key to its map type using the MappingPolicy and the
    // default stream type.
    struct AssociateToMap {
        template <typename Key>
        struct apply {
            typedef typename boost::mpl::apply<
                        MappingPolicy, Key, IOStream>::type Value;
            typedef boost::fusion::pair<Key, Value> type;
        };
    };

    // Zip the Keys with their associated container. This is necessary
    // to create the map below.
    typedef typename boost::mpl::transform<Keys, AssociateToMap>::type Zipped;

    // Create a map from Keys to types determined by the MappingPolicy.
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

private:
    boost::filesystem::path const root_;

public:
    template <typename Source>
    explicit Repository(Source const& root) : root_(root) {
        namespace fs = boost::filesystem;
        if (fs::exists(root_) && !fs::is_directory(root_))
            D2_THROW(InvalidRepositoryPathException());
    }

private:
    template <typename Key>
    boost::filesystem::path path_for(Key const& key) const {
        boost::filesystem::path path(root_);
        return path /= NamingPolicy()(key);
    }

    // Return the stream associated to an instance of a Key.
    template <typename Key>
    struct stream_at {
        typedef typename map_at<Key>::type::mapped_type type;

        type& operator()(Dict& dict, Key const& key) const {
            return map_at<Key>()(dict)[key];
        }

        typedef typename map_at<Key>::const_type::mapped_type const_type;

        const_type& operator()(Dict const& dict, Key const& key) const {
            return map_at<Key>()(dict)[key];
        }
    };

    template <typename Stream, typename Key>
    void open_new(Stream& stream, Key const& key) {
        namespace fs = boost::filesystem;
        BOOST_ASSERT_MSG(!stream.is_open(),
            "opening a stream that is already open");

        fs::path path = path_for(key);
        if (!fs::exists(root_))
            fs::create_directories(root_);
        else if (fs::exists(path) && !fs::is_regular_file(path))
            D2_THROW(StreamApertureException()
                        << TargetFilename(path.c_str()));

        // Try opening an existing file with the same name.
        stream.open(path.c_str());
        if (!stream.is_open())
            // Create a new, empty file otherwise.
            stream.open(path.c_str(), std::ios_base::in |
                                      std::ios_base::out |
                                      std::ios_base::trunc);
        if (!stream)
            D2_THROW(StreamApertureException()
                        << TargetFilename(path.c_str()));
    }

public:
    template <typename Key>
    typename stream_at<Key>::type& operator[](Key const& key) {
        typedef typename stream_at<Key>::type Stream;
        Stream& stream = stream_at<Key>()(streams_, key);
        if (!stream.is_open())
            open_new(stream, key);
        return stream;
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
