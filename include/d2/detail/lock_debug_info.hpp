/**
 * This file defines the `StackFrame` and the `LockDebugInfo` classes.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <d2/detail/config.hpp>

#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <iosfwd>
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
    D2_API friend std::ostream& operator<<(std::ostream&, StackFrame const&);

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & ip & function & module;
    }
};

struct LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    typedef std::vector<StackFrame> CallStack;
    CallStack call_stack;

    void init_call_stack(unsigned int ignore = 0);

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b) {
        return a.call_stack == b.call_stack;
    }

    D2_API friend std::istream& operator>>(std::istream&, LockDebugInfo&);
    D2_API friend std::ostream& operator<<(std::ostream&, LockDebugInfo const&);

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & call_stack;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
