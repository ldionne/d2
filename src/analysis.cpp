/**
 * This file implements the graph analysis algorithm.
 */

#define D2_SOURCE
#include <d2/analysis.hpp>
#include <d2/deadlock_diagnostic.hpp>
#include <d2/events.hpp>
#include <d2/lock_graph.hpp>
#include <d2/lock_id.hpp>
#include <d2/segmentation_graph.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/variant.hpp>
#include <iterator>
#include <vector>


namespace d2 {
namespace detail {

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

        std::vector<DeadlockDiagnostic::AcquireStreak> streaks;
        BOOST_FOREACH(EdgeDescriptor const& edge_desc, cycle) {
            Edge const& edge_label = graph[edge_desc];
            LockId const& l1 = graph[source(edge_desc, graph)];
            LockId const& l2 = graph[target(edge_desc, graph)];
            streaks.push_back(
                DeadlockDiagnostic::AcquireStreak(
                    thread_of(edge_label), l1, l2));
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
analyze_lock_ordering(LockGraph const& lg, SegmentationGraph const& sg) {
    std::vector<DeadlockDiagnostic> diagnostics;
    analyze(lg, sg, gather_diagnostics(std::back_inserter(diagnostics)));
    return diagnostics;
}

} // end namespace detail
} // end namespace d2