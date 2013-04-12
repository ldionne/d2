/**
 * This file defines the `StackFrame` and the `LockDebugInfo` classes.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <algorithm>
#include <boost/operators.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <cstddef>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

struct StackFrame : boost::equality_comparable<StackFrame> {
    std::size_t ip;
    std::string function;
    std::string module;

    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & ip & function & module;
    }

    StackFrame()
        : ip(0)
    { }

    StackFrame(std::size_t ip, std::string const& function,
                               std::string const& module)
        : ip(ip), function(function), module(module)
    { }

    friend bool operator==(StackFrame const& a, StackFrame const& b) {
        return a.ip == b.ip && a.function == b.function && a.module == b.module;
    }

    friend std::ostream& operator<<(std::ostream& os, StackFrame const& self) {
        os << self.module << "\t\t\t"
           << (void*)self.ip << "\t\t\t"
           << self.function;
        return os;
    }
};

struct LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    typedef std::vector<StackFrame> CallStack;
    CallStack call_stack;

    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & call_stack;
    }

    void init_call_stack(unsigned int ignore = 0);

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b) {
        return a.call_stack == b.call_stack;
    }

    friend std::ostream&
    operator<<(std::ostream& os, LockDebugInfo const& self) {
        std::copy(self.call_stack.begin(), self.call_stack.end(),
            std::ostream_iterator<StackFrame>(os, "\n"));
        return os;
    }
};
} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
