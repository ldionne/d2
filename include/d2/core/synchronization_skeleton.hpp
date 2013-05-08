/**
 * This file defines the `synchronization_skeleton` class.
 */

#ifndef D2_CORE_SYNCHRONIZATION_SKELETON_HPP
#define D2_CORE_SYNCHRONIZATION_SKELETON_HPP

#include <d2/core/diagnostic.hpp>
#include <d2/core/filesystem.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/core/lock_id.hpp>
#include <d2/core/segmentation_graph.hpp>
#include <d2/core/thread_id.hpp>
#include <d2/detail/decl.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/move/utility.hpp>
#include <boost/noncopyable.hpp>
#include <boost/range/distance.hpp>
#include <cstddef>
#include <dyno/serializing_stream.hpp>
#include <fstream>
#include <ios>
#include <map>
#include <vector>


namespace d2 {
namespace synchronization_skeleton_detail {
/**
 * Class representing a program stripped from all information unrelated to
 * synchronization.
 */
class synchronization_skeleton : boost::noncopyable {
    typedef boost::function<
                void (core::potential_deadlock const&)
            > DeadlockVisitor;

    typedef dyno::serializing_stream<
                std::ifstream, boost::archive::text_iarchive
            > Stream;

    typedef core::filesystem<Stream> Filesystem;

    typedef std::map<core::ThreadId, core::LockId> InitialGatelocksMap;
    D2_DECL void build_segmentation_graph(Stream&);
    D2_DECL void feed_lock_graph(Stream&, InitialGatelocksMap const&);
    D2_DECL void deadlocks_impl(DeadlockVisitor const&) const;
    D2_DECL void fill_initial_gatelocks(InitialGatelocksMap&);

    Filesystem fs_;
    core::SegmentationGraph sg_;
    core::LockGraph lg_;

public:
    /**
     * Create a `synchronization_skeleton` from the events located on the
     * filesystem rooted at `root`.
     *
     * @warning This may be a resource intensive operation since we have
     *          to build two potentially large graphs.
     *
     * @see `d2::core::filesystem`
     */
    template <typename Path>
    explicit synchronization_skeleton(BOOST_FWD_REF(Path) root)
        : fs_(boost::forward<Path>(root), std::ios::in)
    {
        InitialGatelocksMap initial_gatelocks;
        // The start_join file could be absent if we were analyzing a single
        // thread. See `d2::core::filesystem::start_join_file()` for info.
        if (fs_.start_join_file()) {
            build_segmentation_graph(*fs_.start_join_file());
            fill_initial_gatelocks(*fs_.start_join_file(), initial_gatelocks);
        }

        BOOST_FOREACH(Filesystem::file_entry thread, fs_.thread_files())
            feed_lock_graph(thread.stream(), initial_gatelocks);
    }

    /**
     * Return the number of threads that were spawned in the part of the
     * program captured by the skeleton.
     */
    std::size_t number_of_threads() const {
        return boost::distance(fs_.thread_files());
    }

    /**
     * Return the number of unique locks created in the part of the program
     * captured by the skeleton.
     */
    std::size_t number_of_locks() const {
        return num_vertices(lg_);
    }

    /**
     * Detect potential deadlocks in the part of the program captured by the
     * skeleton.
     *
     * More specifically, analyze the order in which locks are acquired
     * relative to each other in different threads and call a `visitor` on
     * potential deadlocks (of type `d2::core::potential_deadlock`).
     *
     * The analysis tries to minimize false positives, i.e. to yield fiew
     * deadlock states that are unreachable by the program.
     *
     * @warning This operation can be time-consuming if the graphs happen
     *          to be very large.
     */
    template <typename Visitor>
    void on_deadlocks(BOOST_FWD_REF(Visitor) visitor) const {
        deadlocks_impl(DeadlockVisitor(boost::forward<Visitor>(visitor)));
    }

    //! Type of the range returned by the `deadlocks()` method.
    typedef std::vector<core::potential_deadlock> deadlock_range;

    /**
     * Call the `on_deadlocks()` method with a visitor that accumulates the
     * potential deadlocks in a range and then return the range.
     *
     * @see `on_deadlocks()`
     */
    D2_DECL deadlock_range deadlocks() const;

    //! @internal Print a graphviz representation of the lock graph.
    D2_DECL void print_lock_graph(std::ostream&) const;

    //! @internal Print a graphviz representation of the segmentation graph.
    D2_DECL void print_segmentation_graph(std::ostream&) const;
};
} // end namespace synchronization_skeleton_detail

namespace core {
    using synchronization_skeleton_detail::synchronization_skeleton;
}
} // end namespace d2

#endif // !D2_CORE_SYNCHRONIZATION_SKELETON_HPP
