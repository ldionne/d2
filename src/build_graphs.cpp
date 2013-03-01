/**
 * This file implements the construction of the graphs.
 */

#define D2_SOURCE
#include <d2/core/build_lock_graph.hpp>
#include <d2/core/build_segmentation_graph.hpp>
#include <d2/events.hpp>
#include <d2/lock_graph.hpp>
#include <d2/segmentation_graph.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

extern void parse_and_build_seg_graph(std::istream& is,
                                      SegmentationGraph& graph) {
    namespace qi = boost::spirit::qi;

    typedef boost::variant<StartEvent, JoinEvent> Event;

    is.unsetf(std::ios_base::skipws);
    std::string source((std::istream_iterator<char>(is)),
                        std::istream_iterator<char>());

    qi::typed_stream<StartEvent> start;
    qi::typed_stream<JoinEvent> join;

    std::vector<Event> events;
    qi::parse(source.begin(), source.end(), *(start | join), events);

    core::build_segmentation_graph<true>()(events, graph);
}

extern void parse_and_build_lock_graph(std::istream& is, LockGraph& graph) {
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

    core::build_lock_graph<true>(events, graph);
}

} // end namespace detail
} // end namespace d2
