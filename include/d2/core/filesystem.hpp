/**
 * This file defines the `filesystem` class.
 */

#ifndef D2_CORE_FILESYSTEM_HPP
#define D2_CORE_FILESYSTEM_HPP

#include <d2/core/events.hpp>
#include <d2/detail/inherit_constructors.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/move/utility.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <dyno/filesystem.hpp>
#include <ios>
#include <string>


namespace d2 {
namespace filesystem_detail {
/**
 * Functor mapping synchronization related events to filenames
 * on a filesystem.
 *
 * All event types but `start` and `join` events are mapped to a file
 * named after the thread identifier that generated them.
 *
 * `start` and `join` events are all mapped to the same file, which is
 * currently named `start_and_join`.
 */
struct mapping_for_sync_events {
    struct get_thread_id : boost::static_visitor<core::events::thread_id> {
        template <typename Event>
        result_type operator()(BOOST_FWD_REF(Event) event) const {
            return thread_of(boost::forward<Event>(event));
        }
    };

    template <typename Event>
    typename boost::enable_if<
        boost::is_same<
            typename boost::remove_reference<Event>::type,
            core::events::thread_specific
        >,
    std::string>::type operator()(BOOST_FWD_REF(Event) event) const {
        return boost::lexical_cast<std::string>(
                    boost::apply_visitor(
                        get_thread_id(), boost::forward<Event>(event)));
    }

    template <typename Event>
    typename boost::enable_if<
        boost::is_same<Event, core::events::non_thread_specific>,
    std::string>::type operator()(Event const&) const {
        return "start_and_join";
    }
};

using dyno::filesystem_error;
using dyno::filesystem_overwrite;
using dyno::filesystem_overwrite_type;

/**
 * Class storing synchronization related events on a local filesystem.
 *
 * @tparam Stream The type of the streams used to read and write events to
 *                files on the filesystem. Its constructor must be compatible
 *                with that of the standard streams.
 */
template <typename Stream>
class filesystem : public dyno::filesystem<mapping_for_sync_events, Stream> {
    typedef dyno::filesystem<mapping_for_sync_events, Stream> Base;

    struct is_thread_file {
        typedef bool result_type;
        template <typename FileEntry>
        result_type operator()(FileEntry const& entry) const {
            return entry.relative_path() != "start_and_join";
        }
    };

public:
    D2_INHERIT_CONSTRUCTORS(filesystem, Base)

    /**
     * Wrap the operand in the appropriate variant to allow saving
     * heterogeneous objects in the same file and forward to the
     * `dyno::filesystem`.
     */
    template <typename Event>
    typename boost::enable_if<
        core::events::is_thread_specific<
            typename boost::remove_reference<Event>::type
        >,
    void>::type dispatch(BOOST_FWD_REF(Event) event) {
        Base::dispatch(
            core::events::thread_specific(boost::forward<Event>(event)));
    }

    template <typename Event>
    typename boost::disable_if<
        core::events::is_thread_specific<
            typename boost::remove_reference<Event>::type
        >,
    void>::type dispatch(BOOST_FWD_REF(Event) event) {
        Base::dispatch(
            core::events::non_thread_specific(boost::forward<Event>(event)));
    }

    /**
     * Type of the range returned by the non-const version of `thread_files()`.
     */
    typedef boost::filtered_range<
                is_thread_file, typename filesystem::file_range const
            > thread_file_range;

    /**
     * Return a range containing all the files representing threads on
     * the filesystem.
     */
    thread_file_range thread_files() {
        return this->files() | boost::adaptors::filtered(is_thread_file());
    }

    //! Type of the range returned by the const version of `thread_files()`.
    typedef boost::filtered_range<
                is_thread_file, typename filesystem::const_file_range const
            > const_thread_file_range;

    //! Overload of `thread_files()` returning a range of const elements.
    const_thread_file_range thread_files() const {
        return this->files() | boost::adaptors::filtered(is_thread_file());
    }

    /**
     * Return the stream associated to the single file holding `start` and
     * `join` events if it exists.
     *
     * @note It can be the case that no such file exists if no `start`s and
     *       `join`s have been recorded.
     */
    boost::optional<typename filesystem::stream_type&> start_join_file() {
        return (*this)["start_and_join"];
    }

    //! Overload returning the start and join stream for observation only.
    boost::optional<typename filesystem::stream_type const&>
    start_join_file() const {
        return (*this)["start_and_join"];
    }
};
} // end namespace filesystem_detail

namespace core {
    using filesystem_detail::filesystem;
    using filesystem_detail::filesystem_error;
    using filesystem_detail::filesystem_overwrite;
    using filesystem_detail::filesystem_overwrite_type;
}
} // end namespace d2

#endif // !D2_CORE_FILESYSTEM_HPP
