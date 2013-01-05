/**
 * This file implements the `SegmentHopEvent` class.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/segment.hpp>
#include <d2/segment_hop_event.hpp>
#include <d2/thread.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


BOOST_FUSION_ADAPT_STRUCT(
    d2::SegmentHopEvent,
    (d2::Thread, thread)
    (d2::Segment, segment)
)

namespace d2 {

D2_API extern
std::ostream& operator<<(std::ostream& os, SegmentHopEvent const& self) {
    os << "hop thread " << self.thread << " to " << self.segment;
    return os;
}

D2_API extern
std::istream& operator>>(std::istream& is, SegmentHopEvent& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, Segment> segment;
    stream_parser<char, Thread> thread;
    is >> match(
            skip(blank)[
                lit("hop thread") >> thread >> "to" >> segment
            ]
        , self);
    return is;
}

} // end namespace d2
