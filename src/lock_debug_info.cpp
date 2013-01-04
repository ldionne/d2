/**
 * This file implements the d2/detail/lock_debug_info.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/detail/event_io.hpp>

#include <boost/assert.hpp>
#include <dbg/frames.hpp>
#include <dbg/symbols.hpp>
#include <iterator>


namespace d2 {
namespace detail {

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

D2_API void LockDebugInfo::init_call_stack(unsigned int ignore /* = 0 */) {
    dbg::call_stack<100> stack;
    dbg::symdb symbols;
    stack.collect(ignore + 1); // ignore our frame

    StackFrameSink<std::back_insert_iterator<CallStack> >
                                    sink(std::back_inserter(call_stack));
    for (unsigned int frame = 0; frame < stack.size(); ++frame)
        symbols.lookup_function(stack.pc(frame), sink);

    BOOST_ASSERT_MSG(call_stack.size() == stack.size(),
                    "not all the frames from the dbg::call_stack "
                    "were copied to this->call_stack");
}

} // end namespace detail
} // end namespace d2
