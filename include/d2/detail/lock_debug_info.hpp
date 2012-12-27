/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_LOCK_DEBUG_INFO_HPP
#define D2_DETAIL_LOCK_DEBUG_INFO_HPP

#include <boost/operators.hpp>
#include <dbg/frames.hpp>
#include <dbg/symbols.hpp>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>


namespace d2 {
namespace detail {

template <typename OutputIterator>
class FormattingSymSink : public dbg::symsink {
    OutputIterator out_;

public:
    explicit FormattingSymSink(OutputIterator const& out) : out_(out) { }

    virtual void process_function(void const* ip, char const* name,
                                                  char const* module) {
        std::stringstream format;
        format << ip << "    " << name << " in " << module;
        *out_++ = format.str();
    }
};

struct LockDebugInfo : boost::equality_comparable<LockDebugInfo> {
    typedef std::vector<std::string> CallStack;
    CallStack call_stack;

    inline void init_call_stack(unsigned int ignore = 0) {
        dbg::call_stack<100> stack;
        dbg::symdb symbols;
        stack.collect(ignore + 1); // ignore our frame

        typedef FormattingSymSink<std::back_insert_iterator<CallStack> >
                                                                    SymSink;
        SymSink sink(std::back_inserter(call_stack));
        for (unsigned int frame = 0; frame < stack.size(); ++frame)
            symbols.lookup_function(stack.pc(frame), sink);
    }

    friend bool operator==(LockDebugInfo const& a, LockDebugInfo const&b){
        return a.call_stack == b.call_stack;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_LOCK_DEBUG_INFO_HPP
