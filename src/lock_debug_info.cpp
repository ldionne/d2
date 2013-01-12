/**
 * This file implements the d2/detail/lock_debug_info.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/detail/lock_debug_info.hpp>

#include <algorithm>
#include <boost/assert.hpp>
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

D2_API std::istream& operator>>(std::istream& is, StackFrame& self) {
    is >> const_cast<void*&>(self.ip);
    is.get(); // dollar

    char c;
    // reasonable minimum of 70 characters with mangled names
    self.function.reserve(70);
    while (is && (c = is.get()) != '$')
        self.function.push_back(c);

    // reasonable minimum of 70 characters for filenames
    self.module.reserve(70);
    while (is && (c = is.get()) != '$')
        self.module.push_back(c);

    return is;
}

D2_API std::ostream& operator<<(std::ostream& os, LockDebugInfo const& self) {
    os << '[';
    std::copy(self.call_stack.begin(), self.call_stack.end(),
                std::ostream_iterator<StackFrame>(os));
    os << ']';
    return os;
}

D2_API std::istream& operator>>(std::istream& is, LockDebugInfo& self) {
    is.get(); // bracket
    while (is && is.peek() != ']') {
        StackFrame frame;
        is >> frame;
        self.call_stack.push_back(frame);
    }
    if (is)
        is.get(); // bracket

    return is;
}

} // end namespace detail
} // end namespace d2
