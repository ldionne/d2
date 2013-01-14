/**
 * This file implements the d2/detail/lock_debug_info.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/lock_debug_info.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <cstddef>
#include <dbg/frames.hpp>
#include <dbg/symbols.hpp>
#include <iterator>
#include <string>


namespace d2 {
namespace detail {

namespace {
    template <typename OutputIterator>
    class StackFrameSink : public dbg::symsink {
        OutputIterator out_;

    public:
        explicit StackFrameSink(OutputIterator const& out) : out_(out) { }

        virtual void process_function(void const* ip, char const* name,
                                                      char const* module) {
            *out_++ = StackFrame(ip, name, module);
        }
    };
} // end anonymous namespace

void LockDebugInfo::init_call_stack(unsigned int ignore /* = 0 */) {
    dbg::call_stack<100> stack;
    dbg::symdb symbols;
    stack.collect(ignore + 1); // ignore our frame
    call_stack.reserve(stack.size());

    StackFrameSink<std::back_insert_iterator<CallStack> >
                                    sink(std::back_inserter(call_stack));
    for (unsigned int frame = 0; frame < stack.size(); ++frame)
        symbols.lookup_function(stack.pc(frame), sink);

    BOOST_ASSERT_MSG(call_stack.size() == stack.size(),
                    "not all the frames from the dbg::call_stack "
                    "were copied to this->call_stack");
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

        if ((is >> length >> sep) && sep == '|' && length > 0) {
            self.data.resize(length);
            is.read(&self.data[0], length);
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
    is >> const_cast<void*&>(self.ip) >> std::ws
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
    if ((is >> num_frames >> sep) && sep == '~' && num_frames > 0) {
        self.call_stack.reserve(num_frames);
        while (num_frames--) {
            StackFrame frame;
            is >> frame;
            self.call_stack.push_back(frame);
        }
    }

    return is;
}

} // end namespace detail
} // end namespace d2
