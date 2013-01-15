/**
 * This file defines the `StackFrame` and the `LockDebugInfo` classes.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <boost/operators.hpp>
#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

struct StackFrame : boost::equality_comparable<StackFrame> {
    void const* ip;
    std::string function;
    std::string module;

    StackFrame()
        : ip(NULL)
    { }

    StackFrame(void const* ip, std::string const& function,
                               std::string const& module)
        : ip(ip), function(function), module(module)
    { }

    friend bool operator==(StackFrame const& a, StackFrame const& b) {
        return a.ip == b.ip && a.function == b.function && a.module == b.module;
    }

    friend std::istream& operator>>(std::istream&, StackFrame&);
    friend std::ostream& operator<<(std::ostream&, StackFrame const&);
};

struct LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    typedef std::vector<StackFrame> CallStack;
    CallStack call_stack;

    void init_call_stack(unsigned int ignore = 0);

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b) {
        return a.call_stack == b.call_stack;
    }

    friend std::istream& operator>>(std::istream&, LockDebugInfo&);
    friend std::ostream& operator<<(std::ostream&, LockDebugInfo const&);
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
