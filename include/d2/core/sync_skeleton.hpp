/**
 * This file defines the `SyncSkeleton` class.
 */

#ifndef D2_CORE_SYNC_SKELETON_HPP
#define D2_CORE_SYNC_SKELETON_HPP

#include <d2/core/deadlock_diagnostic.hpp>
#include <d2/core/filesystem_dispatcher.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/core/segmentation_graph.hpp>
#include <d2/thread_id.hpp>

#include <boost/foreach.hpp>
#include <boost/range/distance.hpp>
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>


namespace d2 {
namespace sync_skeleton_detail {
extern void parse_and_build_seg_graph(std::istream&, core::SegmentationGraph&);
extern void parse_and_build_lock_graph(std::istream&, core::LockGraph&);
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
template <typename Repository>
class SyncSkeleton;

template <>
class SyncSkeleton<core::Filesystem> {
    typedef core::Filesystem Repository;

    Repository& repository_;
    core::SegmentationGraph segmentation_graph_;
    core::LockGraph lock_graph_;

public:
    /**
     * Construct a skeleton from the data stored on a filesystem.
     *
     * @warning This may be a resource intensive operation since we have
     *          to load the content of the whole filesystem in memory and
     *          build two potentially large graphs.
     */
    explicit SyncSkeleton(Repository& repository) : repository_(repository) {
        BOOST_FOREACH(Repository::file_entry file, repository_.files()) {
            file.stream().seekg(0);
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
        return boost::distance(repository_.files()) - 1;
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

private:
    // Silence MSVC warning C4512: assignment operator could not be generated
    SyncSkeleton& operator=(SyncSkeleton const&) /*= delete*/;
};
} // end namespace sync_skeleton_detail

namespace core {
    using sync_skeleton_detail::SyncSkeleton;
}
} // end namespace d2

#endif // !D2_CORE_SYNC_SKELETON_HPP
