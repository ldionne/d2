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
using boost::begin;
using boost::end;

TEST(logging, log_release_event) {
    std::stringstream repo;

    unsigned short t = 888, l = 999;
    set_event_sink(&repo);
    enable_event_logging();
    notify_release(l, t);
    disable_event_logging();

    std::cout << "Logged event:\n" << repo.str();

    std::vector<event> actual(load_events(repo));
    release_event expected((sync_object(l)), thread(t));
    ASSERT_TRUE(expected == boost::get<release_event>(actual[0]));
}

TEST(logging, log_mixed_events) {
    std::vector<event> events;
    sync_object l1((unsigned)88), l2((unsigned)99);
    thread t1((unsigned)22), t2((unsigned)33);

    events +=
        start_event(t1, t2),
        acquire_event(l1, t1),
            acquire_event(l2, t1),
            release_event(l2, t1),
        release_event(l1, t1),
        join_event(t1, t2)
    ;

    std::stringstream repo;
    set_event_sink(&repo);
    enable_event_logging();
    // We use push_event_impl even though it's an implementation detail
    // because it greatly simplifies our task here.
    std::for_each(begin(events), end(events), d2::detail::push_event_impl);
    disable_event_logging();

    std::cout << "Logged events:\n" << repo.str();

    std::vector<event> logged(load_events(repo));
    ASSERT_TRUE(events == logged);
}

TEST(event_io, parse_acquire_event) {
    std::string input = "123 acquires 456";
    std::string::const_iterator first(begin(input)), last(end(input));
    event_parser<std::string::const_iterator> parser;

    event e;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, e));
    ASSERT_TRUE(first == last);

    ASSERT_TRUE(
        acquire_event(sync_object((unsigned)456), thread((unsigned)123)) ==
        boost::get<acquire_event>(e));
}

TEST(event_io, parse_mixed_events) {
    std::string input = "12 acquires 34 at /foo/bar:2\n"
                        "12 releases 34\n"
                        "56 starts 78\n"
                        "56 joins 78";
    std::string::const_iterator first(begin(input)), last(end(input));
    event_parser<std::string::const_iterator> parser;

    std::vector<event> events;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser % '\n', events));
    ASSERT_TRUE(first == last);

    std::vector<event> expected;
    expected +=
        acquire_event(sync_object((unsigned)34), thread((unsigned)12)),
        release_event(sync_object((unsigned)34), thread((unsigned)12)),
        start_event(thread((unsigned)56), thread((unsigned)78)),
        join_event(thread((unsigned)56), thread((unsigned)78))
    ;

    ASSERT_TRUE(expected == events);
}

TEST(event_io, parse_lock_debug_info) {
    lock_debug_info expected;
    expected.file = "/Foo/Bar/baz";
    expected.line = 20;
    std::string input = "/Foo/Bar/baz:20";
    std::string::const_iterator first(begin(input)), last(end(input));
    lock_debug_info_parser<std::string::const_iterator> parser;

    lock_debug_info info;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, info));
    ASSERT_TRUE(first == last);

    ASSERT_TRUE(expected == info);
}

TEST(event_io, parse_lock_debug_info_with_colon_in_filename) {
    lock_debug_info expected;
    expected.file = "C:/Foo/Bar/:12baz";
    expected.line = 20;
    std::string input = "C:/Foo/Bar/:12baz:20";
    std::string::const_iterator first(begin(input)), last(end(input));
    lock_debug_info_parser<std::string::const_iterator> parser;

    lock_debug_info info;
    ASSERT_TRUE(boost::spirit::qi::parse(first, last, parser, info));
    ASSERT_TRUE(first == last);

    ASSERT_TRUE(expected == info);
}

TEST(event_io, generate_lock_debug_info) {
    typedef std::back_insert_iterator<std::string> Iterator;

    lock_debug_info info;
    info.file = "/Foo/Bar/baz";
    info.line = 20;

    std::string result;
    Iterator out(result);
    lock_debug_info_generator<Iterator> generator;
    ASSERT_TRUE(boost::spirit::karma::generate(out, generator, info));
    ASSERT_EQ("/Foo/Bar/baz:20", result);
}

TEST(event_io, generate_lock_debug_info_with_colon_in_filename) {
    typedef std::back_insert_iterator<std::string> Iterator;

    lock_debug_info info;
    info.file = "C:/Foo/Bar/:12baz";
    info.line = 20;

    std::string result;
    Iterator out(result);
    lock_debug_info_generator<Iterator> generator;
    ASSERT_TRUE(boost::spirit::karma::generate(out, generator, info));
    ASSERT_EQ("C:/Foo/Bar/:12baz:20", result);
}
