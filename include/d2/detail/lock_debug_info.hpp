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

template <typename T>
struct with_implicit_constructor : T {
    template <typename U>
    with_implicit_constructor(U const& u) : T(u) { }
};

template <typename T>
class bounded {
    T value_;

public:
    bounded() { }

    // This is implicit because it is only a thin wrapper around T.
    bounded(T const& t) : value_(t) { }

    operator T const&() const {
        return value_;
    }

    operator T&() {
        return value_;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, bounded const& self) {
        return os << make_bounded_output_sequence(self.value_);
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, bounded& self) {
        return is >> make_bounded_input_sequence(self.value_);
    }
};

struct lock_debug_info {
    std::string file;
    int line;
    typedef std::vector<bounded<std::string> > CallStack;

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
        os << make_bounded_output_sequence(self.file)
           << make_bounded_output_sequence(self.call_stack)
           << self.line;
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, lock_debug_info& self) {
        is >> make_bounded_input_sequence(self.file)
           >> make_bounded_input_sequence(self.call_stack)
           >> self.line;
        return is;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
