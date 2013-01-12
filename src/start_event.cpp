/**
 * This file implements the `StartEvent` event.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/events/start_event.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


namespace d2 {

D2_API std::ostream& operator<<(std::ostream& os, StartEvent const& self) {
    os << self.parent << '~' << self.new_parent << '~' << self.child
                                                                << '~';
    return os;
}

D2_API std::istream& operator>>(std::istream& is, StartEvent& self) {
    using namespace boost::spirit::qi;

    unsigned long parent, new_parent, child;
    is >> match(ulong_ >> '~' >> ulong_ >> '~' >> ulong_ >> '~',
                parent, new_parent, child);
    self = StartEvent();
    self.parent += parent;
    self.new_parent += new_parent;
    self.child += child;
    return is;
}

} // end namespace d2
