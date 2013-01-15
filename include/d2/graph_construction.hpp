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
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


namespace d2 {

namespace detail {
template <typename SegmentationGraph>
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

template <typename LockGraph>
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
} // end namespace detail

/**
 * Build the lock graph and the segmentation graph from the events inside the
 * specified `repository`.
 */
template <typename Policy1, typename Policy2,
          typename LockGraph, typename SegmentationGraph>
void build_graphs(EventRepository<Policy1, Policy2>& repository,
                  LockGraph& lock_graph,
                  SegmentationGraph& seg_graph) {
    typedef EventRepository<Policy1, Policy2> EventRepo;
    typedef typename EventRepo::thread_stream_range ThreadStreams;

    detail::parse_and_build_seg_graph(
        repository[EventRepo::process_wide], seg_graph);

    ThreadStreams thread_streams = repository.thread_streams();
    typename ThreadStreams::iterator thread(thread_streams.begin()),
                                     last(thread_streams.end());
    for (; thread != last; ++thread)
        detail::parse_and_build_lock_graph(*thread, lock_graph);
}

} // end namespace d2

#endif // !D2_GRAPH_CONSTRUCTION_HPP
