/**
 * This file implements the `ReleaseEvent` event.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/events/release_event.hpp>

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

D2_API extern std::ostream& operator<<(std::ostream& os,
                                       ReleaseEvent const& self) {
    os << self.thread << ';' << self.lock << ';';
    return os;
}

D2_API extern std::istream& operator>>(std::istream& is, ReleaseEvent& self) {
    using namespace boost::spirit::qi;

    is >> match(ulong_ >> ';' >> ulong_ >> ';', self.thread, self.lock);
    return is;
}

} // end namespace d2
