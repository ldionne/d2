/**
 * This file implements the event logging.
 */

#define D2_SOURCE
#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/config.hpp>
#include <d2/detail/event_io.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/events.hpp>

#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <cstddef>
#include <dbg/frames.hpp>
#include <dbg/symbols.hpp>
#include <iostream>
#include <iterator>
#include <vector>


namespace d2 {
namespace detail {

static basic_mutex sink_lock;
static bool event_logging_enabled = false;
static std::ostream* event_sink = NULL;
static event_generator<std::ostream_iterator<char> > generate_event;

extern void D2_DECL push_event_impl(Event const& e) {
    sink_lock.lock();
    BOOST_ASSERT_MSG(event_logging_enabled,
                        "pushing an event while event logging is disabled");
    BOOST_ASSERT_MSG(event_sink != NULL,
                                "logging events in an invalid NULL sink");

    bool success = boost::spirit::karma::generate(
                    std::ostream_iterator<char>(*event_sink),
                    generate_event << '\n',
                    e);
    (void)success;

    BOOST_ASSERT_MSG(success,
                "unable to generate the event using the karma generator");
    sink_lock.unlock();
}

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

void D2_DECL LockDebugInfo::init_call_stack(unsigned int ignore /* = 0 */) {
    dbg::call_stack<100> stack;
    dbg::symdb symbols;
    stack.collect(ignore + 1); // ignore our frame

    StackFrameSink<std::back_insert_iterator<CallStack> >
                                    sink(std::back_inserter(call_stack));
    for (unsigned int frame = 0; frame < stack.size(); ++frame)
        symbols.lookup_function(stack.pc(frame), sink);
}

} // end namespace detail

extern void D2_DECL set_event_sink(std::ostream* sink) {
    BOOST_ASSERT_MSG(sink != NULL, "setting an invalid NULL sink");
    detail::sink_lock.lock();
    detail::event_sink = sink;
    detail::sink_lock.unlock();
}

extern void D2_DECL disable_event_logging() {
    detail::sink_lock.lock();
    detail::event_logging_enabled = false;
    detail::sink_lock.unlock();
}

extern void D2_DECL enable_event_logging() {
    detail::sink_lock.lock();
    detail::event_logging_enabled = true;
    detail::sink_lock.unlock();
}

extern bool D2_DECL is_enabled() {
    detail::sink_lock.lock();
    bool const enabled = detail::event_logging_enabled;
    detail::sink_lock.unlock();
    return enabled;
}

extern std::vector<Event> D2_DECL load_events(std::istream& source) {
    detail::event_parser<std::string::const_iterator> parse_event;
    source.unsetf(std::ios::skipws);
    std::string const input((std::istream_iterator<char>(source)),
                             std::istream_iterator<char>());

    std::vector<Event> events;
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    bool success = boost::spirit::qi::parse(first, last,
                                            *(parse_event >> '\n'), events);
    (void)success;
    BOOST_ASSERT_MSG(success && first == last,
                            "unable to parse events using the qi grammar");

    return events;
}

} // end namespace d2
