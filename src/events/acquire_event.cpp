/**
 * This file implements the `AcquireEvent` event.
 */

#define D2_SOURCE
#include <d2/events/acquire_event.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


namespace d2 {

extern std::ostream& operator<<(std::ostream& os, AcquireEvent const& self) {
    os << self.thread << '?' << self.lock << '?' << self.info;
    return os;
}

extern std::istream& operator>>(std::istream& is, AcquireEvent& self) {
    using namespace boost::spirit::qi;

    is >> match(ulong_ >> '?' >> ulong_ >> '?', self.thread, self.lock)
       >> self.info;
    return is;
}

} // end namespace d2
