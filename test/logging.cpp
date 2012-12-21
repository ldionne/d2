/**
 * This file contains unit tests for the logging of events.
 */

#include <d2/detail/event_io.hpp>
#include <d2/logging.hpp>

#include <boost/assign.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>


using namespace d2;
using namespace d2::detail;
using namespace boost::assign;

TEST(event_io, parse_lock_debug_info_without_call_stack) {
    lock_debug_info expected;
    expected.file = "/Foo/Bar/baz";
    expected.line = 20;
    std::string input = "[[/Foo/Bar/baz]][[20]]";
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    lock_debug_info_parser<std::string::const_iterator> parser;

    lock_debug_info info;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, info));
    ASSERT_TRUE(first == last);

    ASSERT_TRUE(expected == info);
}

TEST(event_io, generate_lock_debug_info_without_call_stack) {
    typedef std::back_insert_iterator<std::string> Iterator;

    lock_debug_info info;
    info.file = "/Foo/Bar/baz";
    info.line = 20;

    std::string result;
    Iterator out(result);
    lock_debug_info_generator<Iterator> generator;
    ASSERT_TRUE(boost::spirit::karma::generate(out, generator, info));
    ASSERT_EQ("[[/Foo/Bar/baz]][[20]]", result);
}

TEST(event_io, generate_lock_debug_info_with_call_stack) {
    typedef std::back_insert_iterator<std::string> Iterator;

    lock_debug_info info;
    info.file = "/Foo/Bar/baz";
    info.line = 20;
    info.call_stack += "frame1", "frame2";

    std::string result;
    Iterator out(result);
    lock_debug_info_generator<Iterator> generator;
    ASSERT_TRUE(boost::spirit::karma::generate(out, generator, info));
    ASSERT_EQ("[[/Foo/Bar/baz]][[20]][[frame1\nframe2]]", result);
}

TEST(event_io, parse_lock_debug_info_with_call_stack) {
    lock_debug_info expected;
    expected.file = "/Foo/Bar/baz";
    expected.line = 20;
    expected.call_stack += "frame1", "frame2";
    std::string input = "[[/Foo/Bar/baz]][[20]][[frame1\nframe2]]";
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    lock_debug_info_parser<std::string::const_iterator> parser;

    lock_debug_info info;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, info));
    ASSERT_TRUE(first == last);

    ASSERT_TRUE(expected == info);
}

TEST(event_io, parse_acquire_event) {
    std::string input = "123 acquires 456";
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    event_parser<std::string::const_iterator> parser;

    Event e;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, e));
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
    // We use push_event_impl even though it's an implementation detail
    // because it greatly simplifies our task here.
    std::for_each(boost::begin(events), boost::end(events),
                                                d2::detail::push_event_impl);
    disable_event_logging();

    std::cout << "Logged events:\n" << repo.str();

    std::vector<Event> logged(load_events(repo));
    ASSERT_TRUE(events == logged);
}
