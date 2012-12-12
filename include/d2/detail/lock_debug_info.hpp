/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <d2/btrace/call_stack.hpp>
#include <d2/detail/bounded_io_sequence.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <cstddef>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

struct lock_debug_info {
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
        os << make_bounded_output_sequence(self.file);

        os << self.call_stack.size() << ':';
        CallStack::const_iterator it(self.call_stack.begin()),
                                  last(self.call_stack.end());
        for (; it != last; ++it)
            os << make_bounded_output_sequence(*it);

        os << self.line;
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, lock_debug_info& self) {
        is >> make_bounded_input_sequence(self.file);

        CallStack::size_type size;
        char colon;
        is >> size >> colon;
        while (size--) {
            std::string s;
            is >> make_bounded_input_sequence(s);
            self.call_stack.push_back(s);
        }
        is >> self.line;
        return is;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
