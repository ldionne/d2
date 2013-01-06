/**
 * This file implements the `AcquireEvent` class.
 */

#define D2_SOURCE
#include <d2/acquire_event.hpp>
#include <d2/detail/config.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


BOOST_FUSION_ADAPT_STRUCT(
    d2::AcquireEvent,
    (d2::Thread, thread)
    (d2::SyncObject, lock)
    (d2::detail::LockDebugInfo, info)
)

namespace d2 {

D2_API extern
std::ostream& operator<<(std::ostream& os, AcquireEvent const& self) {
    os << self.thread << " acquires " << self.lock << " at " << self.info;
    return os;
}

D2_API extern
std::istream& operator>>(std::istream& is, AcquireEvent& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, Thread> thread;
    stream_parser<char, SyncObject> lock;
    stream_parser<char, detail::LockDebugInfo> info;
    is >> match(
            skip(blank)[thread >> lit("acquires") >> lock >> "at" >> info]
        , self);
    return is;
}

} // end namespace d2
