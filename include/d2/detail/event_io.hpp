/**
 * This file defines means to perform input/output of events.
 */

#ifndef D2_DETAIL_EVENT_IO_HPP
#define D2_DETAIL_EVENT_IO_HPP

#include <d2/events.hpp>
#include <d2/types.hpp>

#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/phoenix.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>


namespace d2 {
namespace detail {

template <typename Iterator>
struct event_parser : boost::spirit::qi::grammar<Iterator, event()> {
    event_parser() : event_parser::base_type(one_event) {
        using namespace boost::spirit::qi;
        namespace phx = boost::phoenix;

        one_event %= acquire | release | start | join;

        acquire
            =   skip(space)
            [
                (parse_thread >> "acquires" >> parse_sync_object)
                [
                    _val = phx::construct<acquire_event>(_2, _1)
                ]
            ]
            ;

        release
            =   skip(space)
            [
                (parse_thread >> "releases" >> parse_sync_object)
                [
                    _val = phx::construct<release_event>(_2, _1)
                ]
            ]
            ;

        start
            =   skip(space)
            [
                (parse_thread >> "starts" >> parse_thread)
                [
                    _val = phx::construct<start_event>(_1, _2)
                ]
            ]
            ;

        join
            =   skip(space)
            [
                (parse_thread >> "joins" >> parse_thread)
                [
                    _val = phx::construct<join_event>(_1, _2)
                ]
            ]
            ;

        parse_sync_object %= stream_parser<char, sync_object>();

        parse_thread %= stream_parser<char, thread>();
    }

private:
    boost::spirit::qi::rule<Iterator, event()> one_event;
    boost::spirit::qi::rule<Iterator, acquire_event()> acquire;
    boost::spirit::qi::rule<Iterator, release_event()> release;
    boost::spirit::qi::rule<Iterator, start_event()> start;
    boost::spirit::qi::rule<Iterator, join_event()> join;
    boost::spirit::qi::rule<Iterator, sync_object()> parse_sync_object;
    boost::spirit::qi::rule<Iterator, thread()> parse_thread;
};


template <typename Iterator>
struct event_generator : boost::spirit::karma::grammar<Iterator, event()> {

    event_generator() : event_generator::base_type(one_event) {
        using namespace boost::spirit::karma;

        one_event = (acquire | release | start | join);

        acquire
            =   (stream << " acquires " << stream)
            [
                _1 = &_val->*&acquire_event::thread,
                _2 = &_val->*&acquire_event::lock
            ]
            ;

        release
            =   (stream << " releases " << stream)
            [
                _1 = &_val->*&release_event::thread,
                _2 = &_val->*&release_event::lock
            ]
            ;

        start
            =   (stream << " starts " << stream)
            [
                _1 = &_val->*&start_event::parent,
                _2 = &_val->*&start_event::child
            ]
            ;

        join
            =   (stream << " joins " << stream)
            [
                _1 = &_val->*&join_event::parent,
                _2 = &_val->*&join_event::child
            ]
            ;
    }

private:
    boost::spirit::karma::rule<Iterator, event()> one_event;
    boost::spirit::karma::rule<Iterator, acquire_event()> acquire;
    boost::spirit::karma::rule<Iterator, release_event()> release;
    boost::spirit::karma::rule<Iterator, start_event()> start;
    boost::spirit::karma::rule<Iterator, join_event()> join;
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_EVENT_IO_HPP
