/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <d2/detail/config.hpp>

#include <algorithm>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/operators.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <iterator>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

struct StackFrame : boost::equality_comparable<StackFrame> {
    void const* ip;
    std::string function;
    std::string module;

    inline StackFrame() { }

    inline StackFrame(void const* ip, std::string const& function,
                                      std::string const& module)
        : ip(ip), function(function), module(module)
    { }

    friend bool operator==(StackFrame const& a, StackFrame const& b) {
        return a.ip == b.ip && a.function == b.function && a.module==b.module;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, StackFrame const& self) {
        os << self.ip << "    " << self.function << " in " << self.module;
        return os;
    }
};
} // end namespace detail
} // end namespace d2

BOOST_FUSION_ADAPT_STRUCT(
    d2::detail::StackFrame,
    (void const*, ip)
    (std::string, function)
    (std::string, module)
)

namespace d2 {
namespace detail {

struct LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    typedef std::vector<StackFrame> CallStack;
    CallStack call_stack;

    D2_API void init_call_stack(unsigned int ignore = 0);

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b){
        return a.call_stack == b.call_stack;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, LockDebugInfo const& self) {
        std::copy(boost::begin(self.call_stack), boost::end(self.call_stack),
                  std::ostream_iterator<StackFrame>(os, "\n"));
        return os;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
