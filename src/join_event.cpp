/**
 * This file implements the `JoinEvent` class.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/join_event.hpp>
#include <d2/segment.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


BOOST_FUSION_ADAPT_STRUCT(
    d2::JoinEvent,
    (d2::Segment, parent)
    (d2::Segment, new_parent)
    (d2::Segment, child)
)

namespace d2 {

D2_API extern
std::ostream& operator<<(std::ostream& os, JoinEvent const& self) {
    os << "join (" << self.parent << ", "
                   << self.new_parent << ", "
                   << self.child << ')';
    return os;
}

D2_API extern
std::istream& operator>>(std::istream& is, JoinEvent& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, Segment> segment;
    is >> match(
        skip(blank)[
          lit("join (") >> segment >> ',' >> segment >> ',' >> segment >> ')'
        ]
        , self);
    return is;
}

} // end namespace d2
