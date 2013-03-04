/**
 * This file defines the `synchronization_skeleton` class.
 */

#ifndef D2_CORE_SYNCHRONIZATION_SKELETON_HPP
#define D2_CORE_SYNCHRONIZATION_SKELETON_HPP

#include <d2/core/filesystem.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/core/segmentation_graph.hpp>

#include <boost/move/utility.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/range/distance.hpp>
#include <cstddef>
#include <ios>


namespace d2 {
namespace synchronization_skeleton_detail {
/**
 * Class representing a program stripped from all information unrelated to
 * synchronization.
 */
template <typename Storage>
class synchronization_skeleton {
public:
    /**
     * Inform the skeleton that an `event` just happened.
     *
     * Typically, the skeleton will store the event, but it could also
     * discard it or something else. The correct behavior will depend
     * on the particular implementation details of a skeleton.
     */
    template <typename Event>
    void feed(BOOST_FWD_REF(Event));

    /**
     * Return the number of threads that were spawned in the part of the
     * program captured by the skeleton.
     */
    std::size_t number_of_threads() const;
};

/**
 * Implementation of a `synchronization_skeleton` storing its data on a
 * filesystem.
 */
template <typename Stream>
class synchronization_skeleton<core::filesystem<Stream> >
    : boost::noncopyable
{
    typedef core::filesystem<Stream> Storage;

    Storage fs_;

    struct in_memory_representation {
        core::SegmentationGraph sg_;
        core::LockGraph lg_;
    };
    boost::optional<in_memory_representation> analysis_data_;

public:
    /**
     * Create a filesystem-based `synchronization_skeleton` whose filesystem
     * is located at `root`.
     *
     * Upon creation, the filesystem is also passed the `mode` open modes.
     *
     * @see `core::filesystem`
     */
    template <typename Path>
    synchronization_skeleton(BOOST_FWD_REF(Path) root, std::ios::openmode mode)
        : fs_(boost::forward<Path>(root), mode)
    { }

    /**
     * Create a filesystem-based `synchronization_skeleton` whose filesystem
     * is located at `root`. Anything located at `root` is removed before
     * the filesystem is created.
     *
     * @see `core::filesystem`
     */
    template <typename Path>
    synchronization_skeleton(BOOST_FWD_REF(Path) root,
                             core::filesystem_overwrite_type const& flag)
        : fs_(boost::forward<Path>(root), flag)
    { }

    //! @internal Store the event on the underlying filesystem.
    template <typename Event>
    void feed(BOOST_FWD_REF(Event) event) {
        fs_.dispatch(boost::forward<Event>(event));
    }

    /**
     * @internal
     * Return the number of files representing threads on the filesystem.
     */
    std::size_t number_of_threads() const {
        return boost::distance(fs_.thread_files());
    }
};
} // end namespace synchronization_skeleton_detail

namespace core {
    using synchronization_skeleton_detail::synchronization_skeleton;
}
} // end namespace d2

#endif // !D2_CORE_SYNCHRONIZATION_SKELETON_HPP
