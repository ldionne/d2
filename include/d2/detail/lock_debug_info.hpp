/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <d2/btrace/call_stack.hpp>
#include <d2/detail/bounded_io_sequence.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/operators.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <cstddef>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

struct lock_debug_info : boost::equality_comparable<lock_debug_info> {
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

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, lock_debug_info const& self) {
        os << make_bounded_output_sequence(self.file) << self.line;
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, lock_debug_info& self) {
        is >> make_bounded_input_sequence(self.file) >> self.line;
        return is;
    }

    friend bool operator==(lock_debug_info const& a, lock_debug_info const&b){
        return a.file == b.file && a.line == b.line &&
               a.call_stack == b.call_stack;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
