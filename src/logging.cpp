/**
 * This file implements the event logging.
 */

#include <d2/detail/basic_mutex.hpp>
#include <d2/events.hpp>

#define BOOST_SPIRIT_DEBUG 0
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/assert.hpp>
#include <boost/phoenix.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>


namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;
namespace phx = boost::phoenix;
namespace fsn = boost::fusion;

namespace d2 {
namespace detail {

template <typename Iterator>
struct event_parser : qi::grammar<Iterator, event()> {
    event_parser() : event_parser::base_type(one_event) {
        using namespace qi;

        one_event %= skip(ascii::space)[acquire | release | start | join];

        acquire
            =   (parse_thread >> lit("acquires") >> parse_sync_object)
            [
                _val = phx::construct<acquire_event>(_2, _1)
            ]
            ;

        release
            =   (parse_thread >> lit("releases") >> parse_sync_object)
            [
                _val = phx::construct<release_event>(_2, _1)
            ]
            ;

        start
            =   (parse_thread >> lit("starts") >> parse_thread)
            [
                _val = phx::construct<start_event>(_1, _2)
            ]
            ;

        join
            =   (parse_thread >> lit("joins") >> parse_thread)
            [
                _val = phx::construct<join_event>(_1, _2)
            ]
            ;

        parse_thread = ulong_[_val = phx::construct<thread>(_1)];

        parse_sync_object = ulong_[_val = phx::construct<sync_object>(_1)];

#if BOOST_SPIRIT_DEBUG
        BOOST_SPIRIT_DEBUG_NODE(one_event);
        BOOST_SPIRIT_DEBUG_NODE(acquire);
        BOOST_SPIRIT_DEBUG_NODE(release);
        BOOST_SPIRIT_DEBUG_NODE(start);
        BOOST_SPIRIT_DEBUG_NODE(join);
        BOOST_SPIRIT_DEBUG_NODE(parse_thread);
        BOOST_SPIRIT_DEBUG_NODE(parse_sync_object);
#endif
    }

private:
    qi::rule<Iterator, event()> one_event;
    qi::rule<Iterator, acquire_event()> acquire;
    qi::rule<Iterator, release_event()> release;
    qi::rule<Iterator, start_event()> start;
    qi::rule<Iterator, join_event()> join;
    qi::rule<Iterator, thread()> parse_thread;
    qi::rule<Iterator, sync_object()> parse_sync_object;
};


template <typename Iterator>
struct event_generator : karma::grammar<Iterator, event()> {

    event_generator() : event_generator::base_type(one_event) {
        using namespace karma;

        one_event = acquire | release | start | join;

        acquire
            =   (gen_thread << lit(" acquires ") << gen_sync_object)
            [
                _1 = &_val->*&acquire_event::thread,
                _2 = &_val->*&acquire_event::lock
            ]
            ;

        release
            =   (gen_thread << lit(" releases ") << gen_sync_object)
            [
                _1 = &_val->*&release_event::thread,
                _2 = &_val->*&release_event::lock
            ]
            ;

        start
            =   (gen_thread << lit(" starts ") << gen_thread)
            [
                _1 = &_val->*&start_event::parent,
                _2 = &_val->*&start_event::child
            ]
            ;

        join
            =   (gen_thread << lit(" joins ") << gen_thread)
            [
                _1 = &_val->*&join_event::parent,
                _2 = &_val->*&join_event::child
            ]
            ;

        gen_thread = ulong_[call_unique_id()];

        gen_sync_object = ulong_[call_unique_id()];

#if BOOST_SPIRIT_DEBUG
        BOOST_SPIRIT_DEBUG_NODE(one_event);
        BOOST_SPIRIT_DEBUG_NODE(acquire);
        BOOST_SPIRIT_DEBUG_NODE(release);
        BOOST_SPIRIT_DEBUG_NODE(start);
        BOOST_SPIRIT_DEBUG_NODE(join);
        BOOST_SPIRIT_DEBUG_NODE(gen_thread);
        BOOST_SPIRIT_DEBUG_NODE(gen_sync_object);
#endif
    }

private:
    struct call_unique_id {
        template <typename T>
        std::size_t operator()(T const& t, karma::unused_type,
                                           karma::unused_type) const {
            return unique_id(t);
        }
    };

    karma::rule<Iterator, event()> one_event;
    karma::rule<Iterator, acquire_event()> acquire;
    karma::rule<Iterator, release_event()> release;
    karma::rule<Iterator, start_event()> start;
    karma::rule<Iterator, join_event()> join;
    karma::rule<Iterator, thread()> gen_thread;
    karma::rule<Iterator, sync_object()> gen_sync_object;
};


static basic_mutex sink_lock;
static bool is_enabled = false;
static std::ostream* event_sink = NULL;
static event_generator<std::ostream_iterator<char> > generate_event;

extern void push_event_impl(event const& e) {
    sink_lock.lock();
    if (is_enabled) {
        BOOST_ASSERT_MSG(event_sink != NULL,
                                    "logging events in an invalid NULL sink");

        bool success = karma::generate(
                        std::ostream_iterator<char>(*event_sink),
                        generate_event << '\n',
                        e);
        (void)success;

        BOOST_ASSERT_MSG(success,
                    "unable to generate the event using the karma generator");
    }
    sink_lock.unlock();
}

} // end namespace detail

extern void set_event_sink(std::ostream* sink) {
    BOOST_ASSERT_MSG(sink != NULL, "setting an invalid NULL sink");
    detail::sink_lock.lock();
    detail::event_sink = sink;
    detail::sink_lock.unlock();
}

extern void disable_event_logging() {
    detail::sink_lock.lock();
    detail::is_enabled = false;
    detail::sink_lock.unlock();
}

extern void enable_event_logging() {
    detail::sink_lock.lock();
    detail::is_enabled = true;
    detail::sink_lock.unlock();
}

extern std::vector<event> load_events(std::istream& source) {
    detail::event_parser<std::string::const_iterator> parse_event;
    std::string const input((std::istream_iterator<char>(source)),
                             std::istream_iterator<char>());

    std::vector<event> events;
    bool success = qi::parse(boost::begin(input), boost::end(input),
                             parse_event % '\n',
                             events);
    (void)success;
    BOOST_ASSERT_MSG(success, "unable to parse events using the qi grammar");

    return events;
}

} // end namespace d2
