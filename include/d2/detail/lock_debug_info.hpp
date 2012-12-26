/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <d2/btrace/call_stack.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/operators.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <cstddef>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

struct LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    std::string file;
    int line;
    typedef std::vector<std::string> CallStack;

    CallStack call_stack;

    inline void init_call_stack(std::size_t max_frames = 100) {
        using namespace boost;
        btrace::dynamic_call_stack cs(max_frames);
        call_stack.reserve(cs.size());
        range::push_back(call_stack, cs | adaptors::transformed(
                        lambda::bind(&btrace::stack_frame::str, lambda::_1)));
    }

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b){
        return a.file == b.file && a.line == b.line &&
               a.call_stack == b.call_stack;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
