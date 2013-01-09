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
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <fstream>
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
    struct concerned_path;
}
typedef boost::error_info<exception_tag::concerned_path, char const*>
                                                                ConcernedPath;

/**
 * Default mapping policy using `boost::unordered_map`s to map keys to values.
 */
struct boost_unordered_map {
    template <typename Key, typename Value>
    struct apply {
        typedef boost::unordered_map<Key, Value> type;
    };
};

/**
 * Mapping policy using no map at all. All instances of the same key type are
 * mapped to the same value.
 *
 * @note Several methods of `Repository` can't be used with this pseudo-map.
 *       Specifically, `items`, `values` and `keys` won't work because
 *       this pseudo-map can't behave like a range.
 */
struct unary_map {
    template <typename Key, typename Value>
    struct apply {
        struct type {
            typedef Value mapped_type;
            typedef Key key_type;

            mapped_type& operator[](key_type const&) {
                return value_ ? *value_ : *(value_ = boost::in_place());
            }

            bool empty() const {
                return !value_;
            }

        private:
            boost::optional<mapped_type> value_;
        };
    };
};

/**
 * Default locking policy providing no synchronization at all.
 */
struct no_synchronization {
    template <typename Category, typename Stream>
    struct apply {
        struct type {
            void lock_category(Category const&) { }
            void unlock_category(Category const&) { }
            void lock_stream(Stream const&) { }
            void unlock_stream(Stream const&) { }
        };
    };
};

/**
 * Class representing a repository into which stuff can be stored.
 */
template <typename Categories,
          typename MappingPolicy = boost_unordered_map,
          typename LockingPolicy = no_synchronization>
class Repository {

    template <typename Category>
    struct Bundle {
        // The category to which this bundle is associated.
        typedef Category category_type;

        // The actual type of the streams owned by this bundle.
        // Note: A policy for deciding input only, output only
        //       or input/output would be nice.
        typedef std::fstream stream_type;

        // The associative container associated to this bundle.
        typedef typename boost::mpl::apply<
                            MappingPolicy, category_type, stream_type
                        >::type map_type;

        // The object synchronizing accesses to this bundle.
        typedef typename boost::mpl::apply<
                            LockingPolicy, category_type, stream_type
                        >::type locker_type;

        map_type map;
        locker_type locker;
    };

    // Associate a Category to its Bundle into a fusion pair.
    struct create_category_bundle {
        template <typename Category>
        struct apply {
            typedef boost::fusion::pair<Category, Bundle<Category> > type;
        };
    };

    // Create a sequence of (Category, Bundle) pairs from which we'll be able
    // to create the fusion map below.
    typedef typename boost::mpl::transform<
                Categories, create_category_bundle
            >::type Zipped;

    // Create a compile-time map from categories (types used as a tag) to
    // their associated bundle.
    typedef typename boost::fusion::result_of::as_map<Zipped>::type BundleMap;

    // Note: Don't try to use this map. Use the accessor below instead!
    BundleMap bundle_map_;

    // Return the bundle associated to a Category in the compile-time map.
    template <typename Category>
    struct bundle_of {
        typedef typename boost::remove_reference<
                    typename boost::fusion::result_of::at_key<
                        BundleMap, Category
                    >::type
                >::type type;

        typedef typename boost::remove_reference<
                    typename boost::fusion::result_of::at_key<
                        BundleMap const, Category
                    >::type
                >::type const_type;

        template <typename Repo>
        type& operator()(Repo& repo) const {
            return boost::fusion::at_key<Category>(repo.bundle_map_);
        }

        template <typename Repo>
        const_type& operator()(Repo const& repo) const {
            return boost::fusion::at_key<Category>(repo.bundle_map_);
        }
    };

    template <typename Category, template <typename Container> class View>
    struct make_view {
        typedef typename bundle_of<Category>::type Bundle;
        typedef typename bundle_of<Category>::const_type ConstBundle;

        typedef typename View<typename Bundle::map_type>::type type;
        typedef typename View<typename ConstBundle::map_type>::type const_type;
    };

public:
    template <typename Category>
    struct key_view
        : make_view<Category, sandbox::key_view>
    { };

    template <typename Category>
    struct value_view
        : make_view<Category, sandbox::value_view>
    { };

    template <typename Category>
    struct item_view
        : make_view<Category, sandbox::item_view>
    { };

    template <typename Category>
    typename item_view<Category>::type items() {
        typedef typename item_view<Category>::type View;
        return View(bundle_of<Category>()(*this).map);
    }

    template <typename Category>
    typename item_view<Category>::const_type items() const {
        typedef typename item_view<Category>::const_type View;
        return View(bundle_of<Category>()(*this).map);
    }

    template <typename Category>
    typename value_view<Category>::type values() {
        typedef typename value_view<Category>::type View;
        return View(bundle_of<Category>()(*this).map);
    }

    template <typename Category>
    typename value_view<Category>::const_type values() const {
        typedef typename value_view<Category>::const_type View;
        return View(bundle_of<Category>()(*this).map);
    }

    template <typename Category>
    typename key_view<Category>::type keys() {
        typedef typename key_view<Category>::type View;
        return View(bundle_of<Category>()(*this).map);
    }

    template <typename Category>
    typename key_view<Category>::const_type keys() const {
        typedef typename key_view<Category>::const_type View;
        return View(bundle_of<Category>()(*this).map);
    }

private:
    boost::filesystem::path const root_;

    template <typename Category>
    boost::filesystem::path category_path_for() const {
        boost::filesystem::path path(root_);
        return path /= typeid(Category).name();
    }

    struct open_category {
        Repository* this_;

        explicit open_category(Repository* this_) : this_(this_) { }

        typedef void result_type;

        template <typename CategoryBundlePair>
        result_type operator()(CategoryBundlePair const&) const {
            namespace fs = boost::filesystem;
            typedef typename CategoryBundlePair::first_type Category;
            typedef typename bundle_of<Category>::type Bundle;
            typedef typename Bundle::map_type AssociativeContainer;

            Bundle& bundle = bundle_of<Category>()(*this_);
            AssociativeContainer& streams = bundle.map;
            BOOST_ASSERT_MSG(streams.empty(),
                "Opening a category that already has some open streams.");

            fs::path path = this_->category_path_for<Category>();
            BOOST_ASSERT_MSG(!fs::exists(path) || fs::is_directory(path),
                "What should be a category directory is not a directory. "
                "Since we're in charge inside the repository, this is a "
                "programming error.");

            // We create a new directory if it does not already exist.
            // If it did exist, we open all the streams that reside inside
            // that category. Otherwise, there are no streams to open inside
            // the category directory (we just created it), so we're done.
            if (!fs::create_directory(path)) {
                fs::directory_iterator first(path), last;
                for (; first != last; ++first) {
                    fs::path file(*first);
                    BOOST_ASSERT_MSG(fs::is_regular_file(file),
                        "For the moment, there should not be anything else "
                        "than regular files inside a category directory.");
                    // Here, we translate a file name that was generated
                    // using a category instance back to it.
                    Category const& category = boost::lexical_cast<Category>(
                                                    file.filename().string());
                    BOOST_ASSERT_MSG(!streams[category].is_open(),
                        "While opening a category, opening a stream that "
                        "we already know of.");
                    this_->open_stream(streams[category], category);
                }
            }

        }
    };

public:
    /**
     * Create a repository at the path described by `root`.
     * The path must either point to nothing or to an existing directory.
     *
     * If the path points to an existing directory, the directory is used
     * as-if it was previously a repository. Otherwise, a new repository
     * is created.
     *
     * If the path is of any other nature, an exception is thrown.
     *
     * @note `Source` may be any type compatible with
     *       `boost::filesystem::path`'s constructor.
     */
    template <typename Source>
    explicit Repository(Source const& root) : root_(root) {
        namespace fs = boost::filesystem;
        if (fs::exists(root_) && !fs::is_directory(root_))
            D2_THROW(InvalidRepositoryPathException()
                        << ConcernedPath(root_.c_str()));
        fs::create_directories(root_);

        boost::fusion::for_each(bundle_map_, open_category(this));
    }

private:
    template <typename Category>
    boost::filesystem::path path_for(Category const& category) const {
        return category_path_for<Category>() /=
                                boost::lexical_cast<std::string>(category);
    }

    /**
     * Open a stream in the given category for the first time.
     *
     * @warning This method does not synchronize its access to `stream`.
     *          It is the caller's responsibility to make sure `stream`
     *          can be modified safely.
     */
    template <typename Stream, typename Category>
    void open_stream(Stream& stream, Category const& category) const {
        namespace fs = boost::filesystem;
        BOOST_ASSERT_MSG(!stream.is_open(),
            "Opening a stream that is already open.");

        fs::path path = path_for(category);
        if (fs::exists(path) && !fs::is_regular_file(path))
            D2_THROW(StreamApertureException()
                        << ConcernedPath(path.c_str()));

        // Try opening an existing file with the same name.
        stream.open(path.c_str(), std::ios_base::in | std::ios_base::out);
        if (!stream.is_open())
            // Create a new, empty file otherwise.
            stream.open(path.c_str(), std::ios_base::in |
                                      std::ios_base::out |
                                      std::ios_base::trunc);
        if (!stream)
            D2_THROW(StreamApertureException()
                        << ConcernedPath(path.c_str()));
    }

    /**
     * Fetch a stream into its category, perform some action on it and then
     * return a reference to it. Accesses to shared structures is synchronized
     * using the locking policy. See below for details.
     */
    template <typename Category, typename F>
    typename bundle_of<Category>::type::stream_type&
    fetch_stream_and_do(Category const& category, F const& f) {
        typedef typename bundle_of<Category>::type Bundle;
        typedef typename Bundle::locker_type Locker;
        typedef typename Bundle::map_type AssociativeContainer;
        typedef typename Bundle::stream_type Stream;

        Bundle& bundle = bundle_of<Category>()(*this);

        Locker& locker = bundle.locker;
        AssociativeContainer& streams = bundle.map;

        // Use the locker to synchronize the map lookup at the category
        // level. The whole category is locked, so it is not possible for
        // another thread to access the associative map at the same time.
        locker.lock_category(category);
        Stream& stream = streams[category];
        locker.unlock_category(category);

        // Use the locker to synchronize the aperture of the stream at the
        // stream level. Only this stream is locked, so it is not possible for
        // another thread to access this stream at the same time, but it is
        // perfectly possible (and okay) if other threads access other streams
        // in the same category (or in other categories).
        locker.lock_stream(stream);
        if (!stream.is_open())
            open_stream(stream, category);
        // Perform some action on the stream while it's synchronized.
        f(stream);
        locker.unlock_stream(stream);

        // Any usage of the stream beyond this point must be synchronized by
        // the caller as needed.
        return stream;
    }

public:
    /**
     * Return the stream associated to an instance of a category.
     *
     * @warning Any access to the returned stream must be synchronized by
     *          the caller as needed.
     */
    template <typename Category>
    typename bundle_of<Category>::type::stream_type&
    operator[](Category const& category) {
        return fetch_stream_and_do(category, boost::phoenix::nothing);
    }

    /**
     * Write `data` to the output stream associated to `category`.
     * This is equivalent to `repository[category] << data`, except
     * the output operation is synchronized internally in an optimal way.
     */
    template <typename Category, typename Data>
    void write(Category const& category, Data const& data) {
        using boost::phoenix::arg_names::arg1;
        fetch_stream_and_do(category, arg1 << data);
    }

    /**
     * Read into `data` from the input stream associated to `category`.
     * This is equivalent to `repository[category] >> data`, except the
     * input operation is synchronized internally in an optimal way.
     */
    template <typename Category, typename Data>
    void read(Category const& category, Data& data) {
        using boost::phoenix::arg_names::arg1;
        fetch_stream_and_do(category, arg1 >> boost::phoenix::ref(data));
    }

private:
    struct category_is_empty {
        Repository const* this_;

        explicit category_is_empty(Repository const* this_) : this_(this_) { }

        typedef bool result_type;

        template <typename CategoryBundlePair>
        result_type operator()(CategoryBundlePair const&) const {
            typedef typename CategoryBundlePair::first_type Category;
            return bundle_of<Category>()(*this_).map.empty();
        }
    };

public:
    /**
     * Return whether there are any open open streams in any category of
     * the repository.
     *
     * @warning Synchronization is the responsibility of the caller.
     */
    bool empty() const {
        return boost::fusion::all(bundle_map_, category_is_empty(this));
    }
};

} // end namespace d2

#endif // !D2_REPOSITORY_HPP
