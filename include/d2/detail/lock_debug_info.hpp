/**
 * This file defines the `StackFrame` and the `LockDebugInfo` classes.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <d2/detail/config.hpp>

#include <algorithm>
#include <boost/operators.hpp>
#include <iosfwd>
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
        return a.ip == b.ip && a.function == b.function && a.module == b.module;
    }

    D2_API friend std::istream& operator>>(std::istream&, StackFrame&);

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, StackFrame const& self) {
        os << self.ip << '$' << self.function << '$' << self.module << '$';
        return os;
    }
};

struct D2_API LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    typedef std::vector<StackFrame> CallStack;
    CallStack call_stack;

    void init_call_stack(unsigned int ignore = 0);

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b) {
        return a.call_stack == b.call_stack;
    }

    D2_API friend std::istream& operator>>(std::istream&, LockDebugInfo&);

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, LockDebugInfo const& self) {
        os << '[';
        std::copy(self.call_stack.begin(), self.call_stack.end(),
                    std::ostream_iterator<StackFrame>(os));
        os << ']';
        return os;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
