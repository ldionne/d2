/**
 * This file defines the `SyncSkeleton` class.
 */

#ifndef D2_SANDBOX_SYNC_SKELETON_HPP
#define D2_SANDBOX_SYNC_SKELETON_HPP

#include <d2/lock_graph.hpp>
#include <d2/sandbox/deadlock_diagnostic.hpp>
#include <d2/segmentation_graph.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <set>
#include <vector>
#include <string>


namespace d2 {
namespace sandbox {

namespace detail {
    void parse_and_build_seg_graph(std::istream&, SegmentationGraph&);
    void parse_and_build_lock_graph(std::istream&, LockGraph&);
    std::vector<DeadlockDiagnostic>
    analyze_lock_ordering(LockGraph const&, SegmentationGraph const&);
} // end namespace detail

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
class SyncSkeleton {
    Repository& repository_;
    SegmentationGraph segmentation_graph_;
    LockGraph lock_graph_;

    /**
     * Build the lock graph and the segmentation graph from the events inside
     * a repository.
     */
    static void build_graphs(Repository& repo, LockGraph& lg,
                                                    SegmentationGraph& sg) {
        detail::parse_and_build_seg_graph(repo[Repository::process_wide], sg);

        typedef typename Repository::thread_stream_range ThreadStreams;
        ThreadStreams thread_streams = repo.thread_streams();
        typename ThreadStreams::iterator thread(boost::begin(thread_streams)),
                                         last(boost::end(thread_streams));
        for (; thread != last; ++thread)
            detail::parse_and_build_lock_graph(*thread, lg);
    }

#if 0
    /**
     * Structure holding the necessary information to locate a single event
     * within the execution of a program.
     */
    struct Coordinates {
        Process process;
        Thread thread;
        Segment segment;
    };
#endif

public:
    /**
     * Construct a skeleton from the data stored in a repository.
     *
     * @warning This may be a resource intensive operation since we have
     *          to load the content of the whole repository in memory to
     *          build two different graphs.
     */
    explicit SyncSkeleton(Repository& repo) : repository_(repo) {
        build_graphs(repository_, lock_graph_, segmentation_graph_);
    }

    /**
     * Return the number of threads spawned in the part of the program
     * captured by the skeleton.
     */
    std::size_t number_of_threads() const {
        return std::distance(boost::begin(repository_.threads()),
                             boost::end(repository_.threads()));
    }

    /**
     * Return the number of unique locks created in the part of the program
     * captured by the skeleton.
     */
    std::size_t number_of_locks() const {
        return num_vertices(lock_graph_);
    }

private:
    typedef typename Repository::template
                    const_key_view<Thread>::type unspecified_range_of_threads;
    typedef std::vector<DeadlockDiagnostic> unspecified_range_of_diagnostics;

public:
    unspecified_range_of_threads threads() const {
        return repository_.template key_view<Thread>();
    }

    /**
     * Perform an analysis of the order in which locks are acquired relative
     * to each other in the part of the program captured by the skeleton, and
     * return a set of deadlock potential diagnostics.
     *
     * @warning This operation can be quite resource intensive because it
     *          involves analyzing graphs that might be very large.
     */
    unspecified_range_of_diagnostics deadlocks() const {
        return detail::analyze_lock_ordering(lock_graph_, segmentation_graph_);
    }

    // template <typename Something, typename SomethingElse>
    // bool happens_before(Something const& a, SomethingElse const& b) const {
    //     Segment s1 = segment_of(a), s2 = segment_of(b);
    //     return happens_before(s1, s2);
    // }

    // bool happens_before(Segment a, Segment b) const {
    //     // Ask the segmentation graph.
    //     return happens_before(a, b, this->segmentation_graph);
    // }
};

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_SYNC_SKELETON_HPP
