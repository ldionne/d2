/**
 * This file defines the `SyncSkeleton` class.
 */

#ifndef D2_CORE_SYNC_SKELETON_HPP
#define D2_CORE_SYNC_SKELETON_HPP

#include <d2/core/deadlock_diagnostic.hpp>
#include <d2/core/filesystem.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/core/segmentation_graph.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/move/utility.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/distance.hpp>
#include <cstddef>
#include <dyno/serializing_stream.hpp>
#include <fstream>
#include <ios>
#include <vector>


namespace d2 {
namespace sync_skeleton_detail {
typedef dyno::serializing_stream<
            std::ifstream,
            boost::archive::text_iarchive
        > StreamType;

extern void parse_and_build_seg_graph(StreamType&, core::SegmentationGraph&);
extern void parse_and_build_lock_graph(StreamType&, core::LockGraph&);
extern std::vector<core::DeadlockDiagnostic>
analyze_lock_ordering(core::LockGraph const&, core::SegmentationGraph const&);

/**
 * Class representing a program stripped down from any other information than
 * the information related to synchronization.
 *
 * @internal We should probably define a very tiny DSEL to query the skeleton
 *           in flexible ways. For example, it is completely redundant to have
 *           a `total_number_of_locks` method AND a `number_of_locks_in`
 *           method. The `number_of_locks_in` method should be able to
 *           accept some kind of query representing a range of `coordinates'
 *           between which the number of locks is to be computed.
 */
class SyncSkeleton : boost::noncopyable {
    typedef core::filesystem<StreamType> Filesystem;

    Filesystem fs_;
    core::SegmentationGraph segmentation_graph_;
    core::LockGraph lock_graph_;

public:
    /**
     * Construct a skeleton from the data stored on a filesystem.
     *
     * @warning This may be a resource intensive operation since we have
     *          to build two potentially large graphs.
     */
    template <typename Path>
    explicit SyncSkeleton(BOOST_FWD_REF(Path) root)
        : fs_(boost::forward<Path>(root), std::ios::in)
    {
        BOOST_FOREACH(Filesystem::file_entry file, fs_.files()) {
            if (file.relative_path() == "process_wide")
                parse_and_build_seg_graph(file.stream(), segmentation_graph_);
            else
                parse_and_build_lock_graph(file.stream(), lock_graph_);
        }
    }

    /**
     * Return the number of threads spawned in the part of the program
     * captured by the skeleton.
     */
    std::size_t number_of_threads() const {
        // Since we keep one file per thread + one file for the process wide
        // events, this is the number of threads:
        return boost::distance(fs_.files()) - 1;
    }

    /**
     * Return the number of unique locks created in the part of the program
     * captured by the skeleton.
     */
    std::size_t number_of_locks() const {
        return num_vertices(lock_graph_);
    }

private:
    typedef std::vector<core::DeadlockDiagnostic>
                                            unspecified_range_of_diagnostics;

public:
    /**
     * Perform an analysis of the order in which locks are acquired relative
     * to each other in the part of the program captured by the skeleton, and
     * return a set of deadlock potential diagnostics.
     *
     * @warning This operation can be quite resource intensive because it
     *          involves analyzing graphs that might be very large.
     */
    unspecified_range_of_diagnostics deadlocks() const {
        return analyze_lock_ordering(lock_graph_, segmentation_graph_);
    }
};
} // end namespace sync_skeleton_detail

namespace core {
    using sync_skeleton_detail::SyncSkeleton;
}
} // end namespace d2

#endif // !D2_CORE_SYNC_SKELETON_HPP
