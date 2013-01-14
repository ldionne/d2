/**
 * This file implements the `AcquireEvent` event.
 */

#define D2_SOURCE
#include <d2/events/acquire_event.hpp>

#include <boost/config.hpp>
#ifdef BOOST_MSVC
#   pragma warning(push)
// Remove: assignment operator could not be generated
//         conditional expression is constant
#   pragma warning(disable: 4512 4127)
#endif
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#ifdef BOOST_MSVC
#   pragma warning(pop)
#endif
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
