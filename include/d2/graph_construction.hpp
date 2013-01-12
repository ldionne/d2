/**
 * This file defines a thin interface to construct both graphs required during
 * the analysis.
 */

#ifndef D2_GRAPH_CONSTRUCTION_HPP
#define D2_GRAPH_CONSTRUCTION_HPP

#include <d2/event_repository.hpp>
#include <d2/event_traits.hpp>
#include <d2/events.hpp>
#include <d2/lock_graph.hpp>
#include <d2/segmentation_graph.hpp>
#include <d2/thread.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/variant.hpp>
#include <vector>


namespace d2 {

/**
 * Build the lock graph and the segmentation graph from the events inside the
 * specified `repository`.
 */
template <typename Policy1, typename Policy2,
          typename LockGraph, typename SegmentationGraph>
void build_graphs(EventRepository<Policy1, Policy2>& repository,
                  LockGraph& lock_graph,
                  SegmentationGraph& seg_graph) {
    namespace qi = boost::spirit::qi;
    typedef boost::variant<StartEvent, JoinEvent> SegmentationEvent;
    typedef EventRepository<Policy1, Policy2> EventRepo;

    qi::typed_stream<AcquireEvent> acquire;
    qi::typed_stream<ReleaseEvent> release;
    qi::typed_stream<StartEvent> start;
    qi::typed_stream<JoinEvent> join;
    qi::typed_stream<SegmentHopEvent> hop;

    std::istream& segmentation_source = repository[EventRepo::process_wide];
    std::vector<SegmentationEvent> seg_events;
    segmentation_source >> qi::match(
            *(start | join)
        , seg_events);

    build_segmentation_graph<>()(seg_events, seg_graph);

    typedef typename EventRepo::template value_view<Thread>::type
                                                                ThreadSources;
    ThreadSources thread_sources = repository.template values<Thread>();
    typename ThreadSources::iterator first(thread_sources.begin()),
                                     last(thread_sources.end());
    for (; first != last; ++first) {
        std::istream& source = *first;

        typedef boost::variant<AcquireEvent, ReleaseEvent, SegmentHopEvent>
                                                                ThreadEvent;
        std::vector<ThreadEvent> thread_events;

        source >> qi::match(
                *(acquire | release | hop)
            , thread_events);

        build_lock_graph<>()(thread_events, lock_graph);
    }
}

} // end namespace d2

#endif // !D2_GRAPH_CONSTRUCTION_HPP
