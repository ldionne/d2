/**
 * This file implements the `deadlocks_impl` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/analysis.hpp>
#include <d2/core/diagnostic.hpp>
#include <d2/core/lock_id.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/detail/decl.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/move/utility.hpp>
#include <boost/phoenix/core/argument.hpp>
#include <boost/phoenix/stl/container.hpp>
#include <boost/ref.hpp>
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

        std::vector<core::deadlocked_thread> threads;
        BOOST_FOREACH(EdgeDescriptor const& edge_desc, cycle) {
            EdgeLabel const& edge_label = graph[edge_desc];

            std::vector<LockId> were_held(
                gatelocks_of(edge_label).template get<1>().begin(),
                gatelocks_of(edge_label).template get<1>().end());

            LockId when_waited_for = graph[target(edge_desc, graph)];

            core::deadlocked_thread thread(
                thread_of(edge_label), boost::move(were_held), when_waited_for
            );
            thread.holding_info[0] = edge_label.l1_info;
            thread.waiting_for_info = edge_label.l2_info;
            threads.push_back(boost::move(thread));
        }

        core::potential_deadlock deadlock(boost::move(threads));
        visitor_(boost::move(deadlock));
    }
};

D2_DECL void
synchronization_skeleton::deadlocks_impl(DeadlockVisitor const& visitor) const
{
    core::analyze(lg_, sg_,
        GiveSynchronizationSemantics<DeadlockVisitor>(visitor));
}

D2_DECL synchronization_skeleton::deadlock_range
synchronization_skeleton::deadlocks() const {
    deadlock_range dl;
    on_deadlocks(boost::phoenix::push_back(
                    boost::ref(dl), boost::phoenix::arg_names::_1));
    return boost::move(dl);
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
