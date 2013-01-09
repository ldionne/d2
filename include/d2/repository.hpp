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
#include <boost/type_traits/remove_reference.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/typed_in_place_factory.hpp>
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
    struct concerned_path;
}
typedef boost::error_info<exception_tag::concerned_path, char const*>
                                                                ConcernedPath;

/**
 * Default mapping policy using `boost::unordered_map`s to map category
 * instances to streams.
 */
struct boost_unordered_map {
    template <typename Category, typename Stream>
    struct apply {
        typedef boost::unordered_map<Category, Stream> type;
    };
};

/**
 * Mapping policy using no map at all. All instances in the same category are
 * mapped to the same stream.
 *
 * @note Several methods can't be used with any category using this mapping.
 *       Specifically, `items`, `values` and `keys` won't work.
 */
struct unary_map {
    template <typename Category, typename Stream>
    struct apply {
        struct type {
            typedef Stream mapped_type;
            typedef Category key_type;

            mapped_type& operator[](key_type const&) {
                return stream_ ? *stream_ : *(stream_ = boost::in_place());
            }

            bool empty() const {
                return !stream_;
            }

        private:
            boost::optional<mapped_type> stream_;
        };
    };
};

/**
 * Class representing a repository into which stuff can be stored.
 */
template <typename Categories_,
          typename MappingPolicy = boost_unordered_map>
class Repository {
    // A policy for deciding input only, output only
    // or input/output would be nice.
    typedef std::ifstream Istream;
    typedef std::ofstream Ostream;
    typedef std::fstream IOStream;

    // Associate a category to its map type using the MappingPolicy and the
    // default stream type.
    struct AssociateToMap {
        template <typename Category>
        struct apply {
            typedef typename boost::mpl::apply<
                                MappingPolicy, Category, IOStream
                    >::type AssociativeContainer;
            typedef boost::fusion::pair<Category, AssociativeContainer> type;
        };
    };

    // Zip the Categories_ with their associated map. This is necessary
    // to create the Categories below.
    typedef typename boost::mpl::transform<Categories_, AssociateToMap>::type
                                                                    Zipped;

    // Create a map from Categories_ to types determined by the MappingPolicy.
    // This fusion map acts like our instance variables, statically mapping
    // types (a.k.a categories) to associative containers of the type
    // [instance of a category -> stream].
    typedef typename boost::fusion::result_of::as_map<Zipped>::type Categories;

    Categories categories_;

    // Return the associative container associated to a Category.
    template <typename Category>
    struct streams_of {
        typedef typename boost::remove_reference<
            typename boost::fusion::result_of::at_key<
                        Categories, Category>::type
        >::type type;

        type& operator()(Categories& category) const {
            return boost::fusion::at_key<Category>(category);
        }

        typedef typename boost::remove_reference<
            typename boost::fusion::result_of::at_key<
                                            Categories const, Category>::type
        >::type const_type;

        const_type& operator()(Categories const& category) const {
            return boost::fusion::at_key<Category>(category);
        }
    };

    template <typename Category, template <typename Container> class View>
    struct make_view {
        typedef typename View<typename streams_of<Category>::type>::type type;
        typedef typename View<typename streams_of<Category>::const_type>::type
                                                                const_type;
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
        return View(streams_of<Category>()(categories_));
    }

    template <typename Category>
    typename item_view<Category>::const_type items() const {
        typedef typename item_view<Category>::const_type View;
        return View(streams_of<Category>()(categories_));
    }

    template <typename Category>
    typename value_view<Category>::type values() {
        typedef typename value_view<Category>::type View;
        return View(streams_of<Category>()(categories_));
    }

    template <typename Category>
    typename value_view<Category>::const_type values() const {
        typedef typename value_view<Category>::const_type View;
        return View(streams_of<Category>()(categories_));
    }

    template <typename Category>
    typename key_view<Category>::type keys() {
        typedef typename key_view<Category>::type View;
        return View(streams_of<Category>()(categories_));
    }

    template <typename Category>
    typename key_view<Category>::const_type keys() const {
        typedef typename key_view<Category>::const_type View;
        return View(streams_of<Category>()(categories_));
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

        template <typename CategoryPair>
        result_type operator()(CategoryPair& category) const {
            namespace fs = boost::filesystem;
            typedef typename CategoryPair::first_type Category;
            typedef typename CategoryPair::second_type Streams;

            Streams& streams = category.second;
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

        boost::fusion::for_each(categories_, open_category(this));
    }

private:
    template <typename Category>
    boost::filesystem::path path_for(Category const& category) const {
        return category_path_for<Category>() /=
                                boost::lexical_cast<std::string>(category);
    }

    template <typename Stream, typename Category>
    void open_stream(Stream& stream, Category const& category) {
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

public:
    /**
     * Return the stream associated to an instance of a category.
     */
    template <typename Category>
    typename streams_of<Category>::type::mapped_type&
    operator[](Category const& category) {
        typedef typename streams_of<Category>::type::mapped_type Stream;
        Stream& stream = streams_of<Category>()(categories_)[category];
        if (!stream.is_open())
            open_stream(stream, category);
        return stream;
    }

private:
    struct category_is_empty {
        typedef bool result_type;

        template <typename CategoryPair>
        result_type operator()(CategoryPair const& category) const {
            return category.second.empty();
        }
    };

public:
    /**
     * Return whether there are any open streams open in any category in
     * the repository.
     */
    bool empty() const {
        return boost::fusion::all(categories_, category_is_empty());
    }
};

} // end namespace d2

#endif // !D2_REPOSITORY_HPP
