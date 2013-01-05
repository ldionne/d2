/**
 * This file implements the `StartEvent` class.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/start_event.hpp>
#include <d2/thread.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


BOOST_FUSION_ADAPT_STRUCT(
    d2::StartEvent,
    (d2::Thread, parent)
    (d2::Thread, child)
)

namespace d2 {

D2_API extern
std::ostream& operator<<(std::ostream& os, StartEvent const& self) {
    os << self.parent << " starts " << self.child;
    return os;
}

D2_API extern
std::istream& operator>>(std::istream& is, StartEvent& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, Thread> thread;
    is >> match(skip(blank)[lexeme[thread] >> "starts" >> thread], self);
    return is;
}

} // end namespace d2
