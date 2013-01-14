/**
 * This file implements the `SegmentHopEvent` event.
 */

#define D2_SOURCE
#include <d2/events/segment_hop_event.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


namespace d2 {

extern std::ostream& operator<<(std::ostream& os, SegmentHopEvent const& self){
    os << self.thread << '>' << self.segment << '>';
    return os;
}

extern std::istream& operator>>(std::istream& is, SegmentHopEvent& self) {
    using namespace boost::spirit::qi;

    unsigned long segment;
    self = SegmentHopEvent();
    is >> match(ulong_ >> '>' >> ulong_ >> '>', self.thread, segment);
    self.segment += segment;
    return is;
}

} // end namespace d2
