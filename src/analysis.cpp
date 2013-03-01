/**
 * This file implements the graph analysis algorithm.
 */

#define D2_SOURCE
#include <d2/core/analysis.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/core/segmentation_graph.hpp>
#include <d2/core/sync_skeleton.hpp>
#include <d2/deadlock_diagnostic.hpp>
#include <d2/events.hpp>
#include <d2/lock_id.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/variant.hpp>
#include <iterator>
#include <vector>


namespace d2 {
namespace sync_skeleton_detail {
template <typename OutputIterator>
class DiagnosticGatherer {
    OutputIterator out_;

public:
    explicit DiagnosticGatherer(OutputIterator const& out) : out_(out) { }

    template <typename EdgePath, typename LockGraph>
    void operator()(EdgePath const& cycle, LockGraph const& graph) {
        typedef typename boost::graph_traits<LockGraph>::edge_descriptor
                                                            EdgeDescriptor;
        typedef typename boost::edge_property_type<LockGraph>::type Edge;

        std::vector<AcquireStreak> streaks;
        BOOST_FOREACH(EdgeDescriptor const& edge_desc, cycle) {
            Edge const& edge_label = graph[edge_desc];
            LockId l1_l2[] = {graph[source(edge_desc, graph)],
                              graph[target(edge_desc, graph)]};
            streaks.push_back(
                AcquireStreak(thread_of(edge_label), &l1_l2[0], &l1_l2[2]));
        }

        *out_++ = DeadlockDiagnostic(boost::begin(streaks),
                                     boost::end(streaks));
    }
};

template <typename OutputIterator>
DiagnosticGatherer<OutputIterator>
gather_diagnostics(OutputIterator const& out) {
    return DiagnosticGatherer<OutputIterator>(out);
}

extern std::vector<DeadlockDiagnostic>
analyze_lock_ordering(core::LockGraph const& lg,
                      core::SegmentationGraph const& sg) {
    std::vector<DeadlockDiagnostic> diagnostics;
    core::analyze(lg, sg, gather_diagnostics(std::back_inserter(diagnostics)));
    return diagnostics;
}
} // end namespace sync_skeleton_detail
} // end namespace d2
