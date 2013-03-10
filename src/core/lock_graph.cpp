/**
 * This file implements the `d2/core/lock_graph.hpp` header.
 */

#define D2_SOURCE
#include <d2/core/lock_graph.hpp>
#include <d2/detail/decl.hpp>

#include <boost/spirit/include/karma.hpp>
#include <ostream>


namespace karma = boost::spirit::karma;

namespace d2 {
namespace lock_graph_detail {
D2_DECL std::ostream& operator<<(std::ostream& os, LockGraphLabel const& self) {
    os << "{thread: " << self.thread_
       << ", lock1 acquired in segment " << self.s1
       << ", lock2 acquired in segment " << self.s2
       << ", gatelocks: {" << karma::format(
                            karma::stream % ", ", gatelocks_of(self)) << "}";
    return os;
}
} // end namespace lock_graph_detail
} // end namespace d2
