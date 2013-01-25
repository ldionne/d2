/**
 * This file implements the `StartEvent` event.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/events/start_event.hpp>

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
                                       StartEvent const& self) {
    os << parent_of(self) << '~' << new_parent_of(self) << '~' << child_of(self)
                                                                << '~';
    return os;
}

D2_API extern std::istream& operator>>(std::istream& is, StartEvent& self) {
    using namespace boost::spirit::qi;

    unsigned long parent, new_parent, child;
    is >> match(ulong_ >> '~' >> ulong_ >> '~' >> ulong_ >> '~',
                parent, new_parent, child);
    self = StartEvent();
    parent_of(self) += parent;
    new_parent_of(self) += new_parent;
    child_of(self) += child;
    return is;
}

} // end namespace d2
