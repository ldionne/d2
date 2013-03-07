/**
 * This file implements the `deadlocks_impl` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/analysis.hpp>
#include <d2/core/diagnostic.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/detail/decl.hpp>
#include <d2/lock_id.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/move/utility.hpp>
#include <vector>


namespace d2 {
namespace synchronization_skeleton_detail {
/**
 * Wrapper translating graph-centric semantics to
 * synchronization-centric semantics.
 */
template <typename Visitor>
struct GiveSynchronizationSemantics {
    Visitor visitor_;

    explicit GiveSynchronizationSemantics(Visitor const& visitor)
        : visitor_(visitor)
    { }

    template <typename EdgePath, typename LockGraph>
    void operator()(EdgePath const& cycle, LockGraph const& graph) {
        typedef typename boost::graph_traits<
                            LockGraph
                        >::edge_descriptor EdgeDescriptor;

        typedef typename boost::edge_property_type<
                            LockGraph
                        >::type EdgeLabel;

        core::potential_deadlock deadlock;
        BOOST_FOREACH(EdgeDescriptor const& edge_desc, cycle) {
            EdgeLabel const& edge_label = graph[edge_desc];
            LockId was_held = graph[source(edge_desc, graph)];
            LockId while_taking = graph[target(edge_desc, graph)];

            std::vector<LockId> held_locks;
            held_locks.push_back(boost::move(was_held));
            held_locks.push_back(boost::move(while_taking));
            // FIXME:
            // Since we don't currently store all the locks that were
            // (maybe) taken in between these two locks, we can't insert
            // them in the diagnostic. This has to be fixed.

            core::deadlocked_thread thread(
                thread_of(edge_label), boost::move(held_locks)
            );
            deadlock.push_back(boost::move(thread));
        }

        visitor_(boost::move(deadlock));
    }
};

D2_DECL void
synchronization_skeleton::deadlocks_impl(DeadlockVisitor const& visitor) const
{
    core::analyze(lg_, sg_,
        GiveSynchronizationSemantics<DeadlockVisitor>(visitor));
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
