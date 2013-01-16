/**
 * This file implements the `d2/sandbox/sync_skeleton.hpp` header.
 */

#define D2_SOURCE
#include <d2/analysis.hpp>
#include <d2/events.hpp>
#include <d2/lock_graph.hpp>
#include <d2/sandbox/deadlock_diagnostic.hpp>
#include <d2/sandbox/sync_skeleton.hpp>
#include <d2/segmentation_graph.hpp>
#include <d2/thread.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


namespace d2 {
namespace sandbox {
namespace detail {

void parse_and_build_seg_graph(std::istream& is, SegmentationGraph& graph) {
    namespace qi = boost::spirit::qi;

    typedef boost::variant<StartEvent, JoinEvent> Event;

    is.unsetf(std::ios_base::skipws);
    std::string source((std::istream_iterator<char>(is)),
                        std::istream_iterator<char>());

    qi::typed_stream<StartEvent> start;
    qi::typed_stream<JoinEvent> join;

    std::vector<Event> events;
    qi::parse(source.begin(), source.end(), *(start | join), events);

    build_segmentation_graph<>()(events, graph);
}

void parse_and_build_lock_graph(std::istream& is, LockGraph& graph) {
    namespace qi = boost::spirit::qi;

    typedef boost::variant<
                AcquireEvent, ReleaseEvent,
                RecursiveAcquireEvent, RecursiveReleaseEvent,
                SegmentHopEvent
            > Event;

    is.unsetf(std::ios_base::skipws);
    std::string source((std::istream_iterator<char>(is)),
                        std::istream_iterator<char>());

    qi::typed_stream<AcquireEvent> acquire;
    qi::typed_stream<ReleaseEvent> release;
    qi::typed_stream<RecursiveAcquireEvent> rec_acquire;
    qi::typed_stream<RecursiveReleaseEvent> rec_release;
    qi::typed_stream<SegmentHopEvent> hop;

    std::vector<Event> events;
    qi::parse(source.begin(), source.end(),
        *(acquire | release | rec_acquire | rec_release | hop)
    , events);

    build_lock_graph<>()(events, graph);
}


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
            SyncObject const& l1 = graph[source(edge_desc, graph)];
            SyncObject const& l2 = graph[target(edge_desc, graph)];
            streaks.push_back(
                DeadlockDiagnostic::AcquireStreak(edge_label.t, l1, l2));
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

std::vector<DeadlockDiagnostic>
analyze_lock_ordering(LockGraph const& lg, SegmentationGraph const& sg) {
    std::vector<DeadlockDiagnostic> diagnostics;
    analyze(lg, sg, gather_diagnostics(std::back_inserter(diagnostics)));
    return diagnostics;
}

} // end namespace detail
} // end namespace sandbox
} // end namespace d2
