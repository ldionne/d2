/**
 * This file implements the d2/detail/lock_debug_info.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/detail/lock_debug_info.hpp>

#include <boost/assert.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <dbg/frames.hpp>
#include <dbg/symbols.hpp>
#include <iterator>
#include <string>


BOOST_FUSION_ADAPT_STRUCT(
    d2::detail::StackFrame,
    (void const*, ip)
    (std::string, function)
    (std::string, module)
)

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

D2_API extern std::ostream& operator<<(std::ostream& os,
                                       StackFrame const& self) {
    using namespace boost::spirit::karma;

    os << format(

            hex             // ip
        <<  '$'
        <<  +~char_('$')    // function
        <<  '$'
        <<  +~char_('\n')   // module
        <<  '\n'

        , self);

    return os;
}

D2_API extern std::istream& operator>>(std::istream& is, StackFrame& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, void*> voidp;
    is >> match(

            as<void*>()[voidp]  // ip
        >>  '$'
        >>  +~char_('$')        // function
        >>  '$'
        >>  +~char_('\n')       // module
        >>  '\n'

        , self);

    return is;
}

D2_API extern std::ostream& operator<<(std::ostream& os,
                                       LockDebugInfo const& self) {
    using namespace boost::spirit::karma;

    os << format("[[" << *stream << "]]", self.call_stack);

    return os;
}

D2_API extern std::istream& operator>>(std::istream& is,
                                       LockDebugInfo& self) {
    using namespace boost::spirit::qi;

    stream_parser<char, StackFrame> stack_frame;
    // Note: The lexeme[] directive is useless because there is no skipper
    //       anyways, but it is required because we can't apply the kleene
    //       star to a stream_parser directly (probably a bug).
    is >> match("[[" >> *lexeme[stack_frame] >> "]]", self.call_stack);

    return is;
}

} // end namespace detail
} // end namespace d2
