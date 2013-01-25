/**
 * This file defines the `Repository` class.
 */

#ifndef D2_REPOSITORY_HPP
#define D2_REPOSITORY_HPP

#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/exceptions.hpp>
#include <d2/sandbox/container_view.hpp>

#include <boost/config.hpp>
#ifdef BOOST_MSVC
#   pragma warning(push)
// Remove: assignment operator could not be generated
//         conditional expression is constant
//         default constructor could not be generated
//         user defined constructor required
#   pragma warning(disable: 4512 4127 4510 4610)
#endif
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
#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
#include <cerrno>
#include <cstdlib> // for NULL
#include <fstream>
#include <ios>
#include <string>
#include <typeinfo>
#ifdef BOOST_MSVC
#   pragma warning(pop)
#endif


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
 * @todo Have a locking policy for a lock during the whole
 *       `fetch_stream_and_do` operation. This would allow the user to use
 *       only 1 lock to synchronize the whole operation.
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
            void lock() { }
            void unlock() { }
        };
    };
};

/**
 * Locking policy providing synchronization by using some provided
 * synchronization object.
 */
template <typename Mutex>
struct synchronize_with {
    template <typename Category, typename Stream>
    struct apply {
        struct type {
            void lock() { mutex_.lock(); }
            void unlock() { mutex_.unlock(); }

        private:
            Mutex mutex_;
        };
    };
};

/**
 * Default stream type policy using `std::fstream`.
 */
struct use_fstream {
    template <typename Category>
    struct apply {
        typedef std::fstream type;
    };
};

/**
 * Default category naming policy using the `typeid` of a type to derive the
 * name of its category.
 */
struct use_typeid {
    template <typename Category>
    static char const* category_path() {
        return typeid(Category).name();
    }
};

/**
 * Class representing a repository into which data can be stored.
 *
 * The repository functions at 2 different levels. First, there is a
 * compile-time map associating C++ types to a policy-defined type.
 *
 * The C++ types used as keys in the compile-time map are called Categories.
 *
 * Second, the policy-defined type must behave like a conventional map from
 * instances of the Category to some unspecified stream type given by the
 * repository when applying the policy.
 *
 * Policies must be MPL-compatible metafunction classes with the following
 * semantics:
 *  - MappingPolicy
 *      Given a `Category` type and a `Stream` type, it must return the type
 *      of a conventional map from instances of `Category` to instances of
 *      `Stream`. A lookup into this map will happen each time data is to be
 *      stored into the repository.
 *  - CategoryLockingPolicy
 *      Given a `Category` type and a `Stream` type, it must return a type
 *      that is `DefaultConstructible` and that has the `lock` and `unlock`
 *      methods. An instance of the type will be kept for each `Category`,
 *      and will be used for locking a whole `Category` when accessing it.
 *  - StreamLockingPolicy
 *      The same as `CategoryLockingPolicy`, but an instance of the type
 *      will be kept for each stream in the runtime map. Instances of the
 *      type will be used as locks for accessing a single stream in the map,
 *      thus providing a more granular locking.
 *  - StreamTypePolicy
 *      Given a `Category` type, it must return a type that will be used as
 *      a stream for the `Category`.
 *  - CategoryNamingPolicy
 *      Given a `Category` type, it must return a string that will be used
 *      as the name of a subdirectory in which all the objects in that
 *      `Category` will be stored.
 */
template <typename Categories,
          typename MappingPolicy = boost_unordered_map,
          typename CategoryLockingPolicy = no_synchronization,
          typename StreamLockingPolicy = no_synchronization,
          typename StreamTypePolicy = use_fstream,
          typename CategoryNamingPolicy = use_typeid>
class Repository : boost::noncopyable {

    template <typename Category>
    struct Bundle {
        // The category to which this bundle is associated.
        typedef Category category_type;

        // The actual type of the streams owned by this bundle.
        typedef typename boost::mpl::apply<
                    StreamTypePolicy, category_type
                >::type stream_type;

        // The object synchronizing accesses to this bundle.
        typedef typename boost::mpl::apply<
                            CategoryLockingPolicy, category_type, stream_type
                        >::type category_locker_type;

        // The object synchronizing accesses to every stream of this bundle.
        // Note: Each stream has a copy of the stream_locker_type to achieve
        //       possibly _very_ granular locking.
        typedef typename boost::mpl::apply<
                            StreamLockingPolicy, category_type, stream_type
                        >::type stream_locker_type;

        // The type of the objects stored in the associative container.
        struct mapped_type {
            stream_locker_type stream_locker;
            stream_type stream;

            typedef Bundle bundle_type;
        };

        // The associative container associated to this bundle.
        typedef typename boost::mpl::apply<
                            MappingPolicy, category_type, mapped_type
                        >::type map_type;

        map_type map;
        category_locker_type category_locker;
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

    // Accessor for the .stream member that is not yet fully bound.
    template <typename MappedType>
    struct stream_accessor_helper
        : sandbox::member_accessor<
            typename MappedType::bundle_type::stream_type MappedType::*,
            &MappedType::stream
        >
    { };

    // Accessor for the .stream member.
    template <typename NextAccessor = sandbox::identity_accessor>
    struct stream_accessor
        : sandbox::rebind_accessor<
            stream_accessor_helper, NextAccessor
        >
    { };

public:
    template <typename Category>
    class key_view {
        typedef typename bundle_of<Category>::type::map_type Map;

    public:
        typedef boost::select_first_range<Map> type;
    };

    template <typename Category>
    struct const_key_view
        : key_view<Category>
    { };

    template <typename Category>
    class const_value_view {
        typedef typename bundle_of<Category>::type::map_type Map;
        typedef boost::select_second_const_range<Map> Values;

    public:
        typedef boost::transformed_range<stream_accessor<>, Values const> type;
    };

    template <typename Category>
    class value_view {
        typedef typename bundle_of<Category>::type::map_type Map;
        typedef boost::select_second_mutable_range<Map> Values;

    public:
        typedef boost::transformed_range<stream_accessor<>, Values> type;
    };

    template <typename Category>
    typename value_view<Category>::type values() {
        // Note: We have to create an lvalue to avoid errors deep in the boost
        //       range code. It has to do with the fact that rvalues are taken
        //       by const reference while lvalues are taken by reference.
        typedef typename bundle_of<Category>::type::map_type Map;
        typedef boost::select_second_mutable_range<Map> Values;
        Values values = bundle_of<Category>()(*this).map
                        | boost::adaptors::map_values;
        return values | boost::adaptors::transformed(stream_accessor<>());
    }

    template <typename Category>
    typename const_value_view<Category>::type values() const {
        return bundle_of<Category>()(*this).map
                    | boost::adaptors::map_values
                    | boost::adaptors::transformed(stream_accessor<>());
    }

    template <typename Category>
    typename key_view<Category>::type keys() {
        return bundle_of<Category>()(*this).map | boost::adaptors::map_keys;
    }

    template <typename Category>
    typename const_key_view<Category>::type keys() const {
        return bundle_of<Category>()(*this).map | boost::adaptors::map_keys;
    }

private:
    boost::filesystem::path const root_;

    template <typename Category>
    boost::filesystem::path category_path_for() const {
        boost::filesystem::path path(root_);
        return path /= CategoryNamingPolicy::
                        template category_path<Category>();
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
                "opening a category that already has some open streams");

            fs::path path = this_->category_path_for<Category>();
            BOOST_ASSERT_MSG(!fs::exists(path) || fs::is_directory(path),
                "what should be a path to nothing or to a category's "
                "directory is a path to something that is not a directory");

            // We create a new directory if it does not already exist.
            // If it did exist, we open all the streams that reside inside
            // that category. Otherwise, there are no streams to open inside
            // the category directory (we just created it), so we're done.
            if (!fs::create_directory(path)) {
                fs::directory_iterator first(path), last;
                for (; first != last; ++first) {
                    fs::path file(*first);
                    BOOST_ASSERT_MSG(fs::is_regular_file(file),
                        "for the moment, there should not be anything else "
                        "than regular files inside a category directory");
                    // Here, we translate a file name that was generated
                    // using a category instance back to it.
                    Category const& category = boost::lexical_cast<Category>(
                                                    file.filename().string());
                    BOOST_ASSERT_MSG(!streams[category].stream.is_open(),
                        "while opening a category, opening a stream that "
                        "we already know of");
                    this_->open_stream(streams[category].stream, category);
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
                        << boost::errinfo_file_name(root_.string().c_str()));
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
    template <typename Category>
    void open_stream(typename bundle_of<Category>::type::stream_type& stream,
                     Category const& category) const {
        namespace fs = boost::filesystem;
        BOOST_ASSERT_MSG(!stream.is_open(),
            "opening a stream that is already open");

        fs::path path = path_for(category);
        if (fs::exists(path) && !fs::is_regular_file(path))
            D2_THROW(StreamApertureException()
                        << boost::errinfo_file_name(path.string().c_str()));

        // Try opening an existing file with the same name.
        stream.open(path.c_str(), std::ios_base::in | std::ios_base::out);
        if (!stream.is_open())
            // Create a new, empty file otherwise.
            stream.open(path.c_str(), std::ios_base::in |
                                      std::ios_base::out |
                                      std::ios_base::trunc);
        if (!stream)
            D2_THROW(StreamApertureException()
                        << boost::errinfo_errno(errno)
                        << boost::errinfo_file_name(path.string().c_str()));
    }

    /**
     * Fetch a stream into its category, perform some action on it and then
     * return a reference to it. Accesses to shared structures is synchronized
     * using the different locking policies. See below for details.
     */
    template <typename Category, typename F>
    typename bundle_of<Category>::type::stream_type&
    fetch_stream_and_do(Category const& category, F const& f) {
        typedef typename bundle_of<Category>::type Bundle;
        typedef typename Bundle::category_locker_type CategoryLocker;
        typedef typename Bundle::map_type AssociativeContainer;

        typedef typename Bundle::mapped_type StreamBundle;
        typedef typename Bundle::stream_locker_type StreamLocker;
        typedef typename Bundle::stream_type Stream;

        Bundle& bundle = bundle_of<Category>()(*this);
        CategoryLocker& category_locker = bundle.category_locker;
        AssociativeContainer& streams = bundle.map;

        // Use the locker to synchronize the map lookup at the category
        // level. The whole category is locked, so it is not possible for
        // another thread to access the associative map at the same time.
        // Note: The usage of a pointer here is required because we can't
        //       initialize the reference inside the scope of the scoped lock.
        StreamBundle* stream_bundle_ptr = NULL;
        {
            detail::scoped_lock<CategoryLocker> lock(category_locker);
            stream_bundle_ptr = &streams[category];
        }
        BOOST_ASSERT(stream_bundle_ptr != NULL);
        StreamLocker& stream_locker = stream_bundle_ptr->stream_locker;
        Stream& stream = stream_bundle_ptr->stream;

        // Use the locker to synchronize the aperture of the stream at the
        // stream level. Only this stream is locked, so it is not possible for
        // another thread to access this stream at the same time, but it is
        // perfectly possible (and okay) if other threads access other streams
        // in the same category (or in other categories).
        {
            detail::scoped_lock<StreamLocker> lock(stream_locker);
            if (!stream.is_open())
                open_stream(stream, category);
            // Perform some action on the stream while it's synchronized.
            f(stream);
        }

        // Any usage of the stream beyond this point must be synchronized by
        // the caller as needed.
        return stream;
    }

public:
    /**
     * Fetch the stream associated to `category` and execute `f` on it,
     * synchronizing optimally access to the stream.
     *
     * @note When `f` is called, the stream on which it is called
     *       (and only that) is locked from other threads. The other streams
     *       in the repository are _NOT_ locked.
     * @note If `f` throws, the stream will be unlocked.
     */
    template <typename Category, typename UnaryFunction>
    void perform(Category const& category, UnaryFunction const& f) {
        fetch_stream_and_do(category, f);
    }

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
        perform(category, boost::phoenix::arg_names::arg1 << data);
    }

    /**
     * Read into `data` from the input stream associated to `category`.
     * This is equivalent to `repository[category] >> data`, except the
     * input operation is synchronized internally in an optimal way.
     */
    template <typename Category, typename Data>
    void read(Category const& category, Data& data) {
        perform(category,
                boost::phoenix::arg_names::arg1 >> boost::phoenix::ref(data));
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

protected:
    /**
     * Typedef so subclasses have a short way of referring to this class.
     */
    typedef Repository Repository_;
};

} // end namespace d2

#endif // !D2_REPOSITORY_HPP
