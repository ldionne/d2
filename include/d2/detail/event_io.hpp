/**
 * This file defines means to perform input/output of events.
 */

#ifndef D2_DETAIL_EVENT_IO_HPP
#define D2_DETAIL_EVENT_IO_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/events.hpp>
#include <d2/types.hpp>

#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/phoenix.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>


namespace d2 {
namespace detail {

namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;

template <typename Iterator>
struct lock_debug_info_parser : qi::grammar<Iterator, lock_debug_info()> {

    lock_debug_info_parser() : lock_debug_info_parser::base_type(start) {
        using namespace qi;

        start
            =   (as_string[*(char_ - line_part)] >> line_part)
            [
                &_val->*&lock_debug_info::file = _1,
                &_val->*&lock_debug_info::line = _2
            ]
            ;

        line_part %= ':' >> uint_;
    }

private:
    qi::rule<Iterator, lock_debug_info()> start;
    qi::rule<Iterator, unsigned int()> line_part;
};

template <typename Iterator>
struct lock_debug_info_generator : karma::grammar<Iterator,lock_debug_info()>{

    lock_debug_info_generator() : lock_debug_info_generator::base_type(start){
        using namespace karma;
        namespace phx = boost::phoenix;

        start
            =   (string << ':' << uint_)
            [
                _1 = &_val->*&lock_debug_info::file,
                _2 = &_val->*&lock_debug_info::line
            ]
            ;
    }

private:
    static std::vector<char> tovector(std::string const& s) {
        std::vector<char> v(s.begin(), s.end());
        return v;
    }

    karma::rule<Iterator, lock_debug_info()> start;
};

template <typename Iterator>
struct event_parser : qi::grammar<Iterator, event()> {
    event_parser() : event_parser::base_type(one_event) {
        using namespace qi;
        namespace phx = boost::phoenix;

        one_event %= acquire | release | start | join;

        acquire
            =   skip(space)
            [
                (parse_thread >> "acquires" >> parse_sync_object)
                [
                    _val = phx::construct<acquire_event>(_2, _1)
                ]
                >> -(info[&_val->*&acquire_event::info = _1])
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
    qi::rule<Iterator, event()> one_event;
    qi::rule<Iterator, acquire_event()> acquire;
    qi::rule<Iterator, release_event()> release;
    qi::rule<Iterator, start_event()> start;
    qi::rule<Iterator, join_event()> join;
    qi::rule<Iterator, sync_object()> parse_sync_object;
    qi::rule<Iterator, thread()> parse_thread;

    lock_debug_info_parser<Iterator> info;
};


template <typename Iterator>
struct event_generator : karma::grammar<Iterator, event()> {

    event_generator() : event_generator::base_type(one_event) {
        using namespace karma;

        one_event = (acquire | release | start | join);

        acquire
            =   (stream << " acquires " << stream << ' ' << info)
            [
                _1 = &_val->*&acquire_event::thread,
                _2 = &_val->*&acquire_event::lock,
                _3 = &_val->*&acquire_event::info
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
    karma::rule<Iterator, event()> one_event;
    karma::rule<Iterator, acquire_event()> acquire;
    karma::rule<Iterator, release_event()> release;
    karma::rule<Iterator, start_event()> start;
    karma::rule<Iterator, join_event()> join;

    lock_debug_info_generator<Iterator> info;
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_EVENT_IO_HPP
