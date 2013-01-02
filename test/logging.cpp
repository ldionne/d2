/**
 * This file contains unit tests for the logging of events.
 */

#include <d2/detail/event_io.hpp>
#include <d2/logging.hpp>

#include <boost/assign.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/variant.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace d2;
using namespace d2::detail;
using namespace boost::assign;

TEST(event_io, generate_lock_debug_info_with_call_stack) {
    typedef std::back_insert_iterator<std::string> Iterator;

    LockDebugInfo info;
    info.call_stack += StackFrame(0x0, "function1", "file1"),
                       StackFrame(0x0, "function2", "file2");

    std::string result;
    Iterator out(result);
    lock_debug_info_generator<Iterator> generator;
    ASSERT_TRUE(boost::spirit::karma::generate(out, generator, info));
    ASSERT_EQ("[[0 function1%%file1\n0 function2%%file2\n]]", result);
}

TEST(event_io, parse_lock_debug_info_with_call_stack) {
    LockDebugInfo expected;
    expected.call_stack += StackFrame(0x0, "function1", "file1"),
                           StackFrame(0x0, "function2", "file2");
    std::string input = "[[0 function1%%file1\n0 function2%%file2\n]]";
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    lock_debug_info_parser<std::string::const_iterator> parser;

    LockDebugInfo info;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, info));
    ASSERT_TRUE(first == last);

    if (expected != info)
        std::cout << "Expected \"" << expected
                  << "\" but got \"" << info << "\"\n";
    ASSERT_TRUE(expected == info);
}

TEST(event_io, parse_acquire_event) {
    std::string input = "123 acquires 456";
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    event_parser<std::string::const_iterator> parser;

    Event e;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, e));
    if (first != last)
        std::cout << "Left to parse:\"" << std::string(first, last) << "\"\n";
    ASSERT_TRUE(first == last);

    ASSERT_TRUE(
        AcquireEvent(SyncObject((unsigned)456), Thread((unsigned)123)) ==
        boost::get<AcquireEvent>(e));
}

TEST(event_io, parse_mixed_events) {
    std::string input = "12 acquires 34\n"
                        "12 releases 34\n"
                        "56 starts 78\n"
                        "56 joins 78";
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    event_parser<std::string::const_iterator> parser;

    std::vector<Event> events;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser % '\n', events));
    if (first != last)
        std::cout << "Left to parse:\"" << std::string(first, last) << "\"\n";
    ASSERT_TRUE(first == last);

    std::vector<Event> expected;
    expected +=
        AcquireEvent(SyncObject((unsigned)34), Thread((unsigned)12)),
        ReleaseEvent(SyncObject((unsigned)34), Thread((unsigned)12)),
        StartEvent(Thread((unsigned)56), Thread((unsigned)78)),
        JoinEvent(Thread((unsigned)56), Thread((unsigned)78))
    ;

    ASSERT_TRUE(expected == events);
}

TEST(logging, log_release_event) {
    std::stringstream repo;

    unsigned short t = 888, l = 999;
    set_event_sink(&repo);
    enable_event_logging();
    notify_release(l, t);
    disable_event_logging();

    std::cout << "Logged event:\n" << repo.str();

    std::vector<Event> actual(load_events(repo));
    ReleaseEvent expected((SyncObject(l)), Thread(t));
    ASSERT_TRUE(expected == boost::get<ReleaseEvent>(actual[0]));
}

namespace {
    struct push_event {
        template <typename Event>
        void operator()(Event const& event) const {
            boost::apply_visitor(as_visitor(), event);
        }

    private:
        struct as_visitor : boost::static_visitor<void> {
            void operator()(d2::AcquireEvent const& event) const {
                d2::notify_acquire(event.lock, event.thread);
            }

            void operator()(d2::ReleaseEvent const& event) const {
                d2::notify_release(event.lock, event.thread);
            }

            void operator()(d2::StartEvent const& event) const {
                d2::notify_start(event.parent, event.child);
            }

            void operator()(d2::JoinEvent const& event) const {
                d2::notify_join(event.parent, event.child);
            }
        };
    };
} // end namespace detail

TEST(logging, log_mixed_events) {
    std::vector<Event> events;
    SyncObject l1((unsigned)88), l2((unsigned)99);
    Thread t1((unsigned)22), t2((unsigned)33);

    events +=
        StartEvent(t1, t2),
        AcquireEvent(l1, t1),
            AcquireEvent(l2, t1),
            ReleaseEvent(l2, t1),
        ReleaseEvent(l1, t1),
        JoinEvent(t1, t2)
    ;

    std::stringstream repo;
    set_event_sink(&repo);
    enable_event_logging();
    boost::for_each(events, push_event());
    disable_event_logging();

    std::cout << "Logged events:\n" << repo.str();

    std::vector<Event> logged(load_events(repo));
    ASSERT_TRUE(events == logged);
}
