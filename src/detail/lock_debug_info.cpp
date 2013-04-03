/**
 * This file implements the d2/detail/lock_debug_info.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/lock_debug_info.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <cstddef>
#include <backward.hpp>
#include <iostream>
#include <iterator>
#include <string>


namespace d2 {
namespace detail {
void LockDebugInfo::init_call_stack(unsigned int ignore /*= 0*/) {
    backward::StackTrace st;
    st.load_here(100);
    backward::TraceResolver tr;
    tr.load_stacktrace(st);

    call_stack.reserve(st.size());
    for (unsigned int frame = 0; frame < st.size(); ++frame) {
        backward::ResolvedTrace trace = tr.resolve(st[frame]);
        call_stack.push_back(
            StackFrame(reinterpret_cast<std::size_t>(trace.addr), trace.object_function, trace.object_filename));
    }
}

namespace {
template <typename String>
class DelimitedString {
    String& data;
    DelimitedString& operator=(DelimitedString const&) /*= delete*/;

public:
    explicit DelimitedString(String& s) : data(s) { }

    friend std::ostream& operator<<(std::ostream& os,
                                    DelimitedString const& self) {
        os << self.data.size() << '|' << self.data;
        return os;
    }

    friend std::istream& operator>>(std::istream& is,
                                    DelimitedString const& self) {
        std::size_t length;
        char sep = 'X';

        if (is >> length >> sep) {
            if (sep == '|') {
                if (length > 0) {
                    self.data.resize(length);
                    is.read(&self.data[0], length);
                }
            } else {
                is.setstate(std::ios_base::failbit);
            }
        }
        return is;
    }
};

template <typename String>
DelimitedString<String> delimit(String& s) {
    return DelimitedString<String>(s);
}
} // end anonymous namespace

extern std::ostream& operator<<(std::ostream& os, StackFrame const& self) {
    os << self.ip << ' '
       << delimit(self.function)
       << delimit(self.module);
    return os;
}

extern std::istream& operator>>(std::istream& is, StackFrame& self) {
    is >> self.ip >> std::ws
       >> delimit(self.function)
       >> delimit(self.module);
    return is;
}

extern std::ostream& operator<<(std::ostream& os, LockDebugInfo const& self) {
    os << self.call_stack.size() << '~';
    std::copy(self.call_stack.begin(), self.call_stack.end(),
                std::ostream_iterator<StackFrame>(os));
    return os;
}

extern std::istream& operator>>(std::istream& is, LockDebugInfo& self) {
    std::size_t num_frames;
    char sep = 'X';

    self.call_stack.clear();
    if (is >> num_frames >> sep) {
        if (sep == '~') {
            if (num_frames > 0) {
                self.call_stack.reserve(num_frames);
                while (num_frames--) {
                    StackFrame frame;
                    is >> frame;
                    self.call_stack.push_back(frame);
                }
            }
        } else {
            is.setstate(std::ios_base::failbit);
        }
    }

    return is;
}

} // end namespace detail
} // end namespace d2
