/**
 * This file implements the `ReleaseEvent` class.
 */

#define D2_SOURCE
#include <d2/release_event.hpp>
#include <d2/detail/config.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


BOOST_FUSION_ADAPT_STRUCT(
    d2::ReleaseEvent,
    (d2::Thread, thread)
    (d2::SyncObject, lock)
)

namespace d2 {

D2_API extern
std::ostream& operator<<(std::ostream& os, ReleaseEvent const& self) {
    os << self.thread << " releases " << self.lock;
    return os;
}

D2_API extern
std::istream& operator>>(std::istream& is, ReleaseEvent& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, Thread> thread;
    stream_parser<char, SyncObject> lock;
    is >> match(skip(blank)[thread >> lit("releases") >> lock], self);
    return is;
}

} // end namespace d2
