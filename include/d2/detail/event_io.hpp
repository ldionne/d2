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
#include <string>


namespace d2 {
namespace detail {

namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;

template <typename Iterator>
struct lock_debug_info_parser : qi::grammar<Iterator, lock_debug_info()> {

    lock_debug_info_parser() : lock_debug_info_parser::base_type(start) {
        using namespace qi;

        start
            =   filename[&_val->*&lock_debug_info::file = _1]
            >>  line[&_val->*&lock_debug_info::line = _1]
            >>  -(call_stack[&_val->*&lock_debug_info::call_stack = _1])
            ;

        filename %= "[[" >> *(char_ - "]]") >> "]]";

        line %= "[[" >> uint_ >> "]]";

        call_stack %= "[[" >> (*(~char_('\n') - "]]") % '\n') >> "]]";
    }

private:
    qi::rule<Iterator, lock_debug_info()> start;
    qi::rule<Iterator, std::string()> filename;
    qi::rule<Iterator, unsigned int()> line;
    qi::rule<Iterator, lock_debug_info::CallStack()> call_stack;
};

template <typename Iterator>
struct lock_debug_info_generator : karma::grammar<Iterator,lock_debug_info()>{

    lock_debug_info_generator() : lock_debug_info_generator::base_type(start){
        using namespace karma;
        namespace phx = boost::phoenix;

        start
            =   filename[_1 = &_val->*&lock_debug_info::file]
            <<  line[_1 = &_val->*&lock_debug_info::line]
            <<  (-call_stack)[
                phx::if_(!phx::bind(&lock_debug_info::CallStack::empty,
                                    &_val->*&lock_debug_info::call_stack))[
                    phx::arg_names::arg1 =&_val->*&lock_debug_info::call_stack
                ]
                ]
            ;

        filename %= "[[" << string << "]]";

        line %= "[[" << uint_ << "]]";

        call_stack %= "[[" << (string % '\n') << "]]";
    }

private:
    karma::rule<Iterator, lock_debug_info()> start;
    karma::rule<Iterator, std::string()> filename;
    karma::rule<Iterator, unsigned int()> line;
    karma::rule<Iterator, lock_debug_info::CallStack()> call_stack;
};

template <typename Iterator>
struct event_parser : qi::grammar<Iterator, Event()> {
    event_parser() : event_parser::base_type(one_event) {
        using namespace qi;
        namespace phx = boost::phoenix;

        one_event %= acquire | release | start | join;

        acquire
            =   skip(blank)
            [
                (parse_thread >> "acquires" >> parse_sync_object)
                [
                    _val = phx::construct<AcquireEvent>(_2, _1)
                ]
                >> -(info[&_val->*&AcquireEvent::info = _1])
            ]
            ;

        release
            =   skip(blank)
            [
                (parse_thread >> "releases" >> parse_sync_object)
                [
                    _val = phx::construct<ReleaseEvent>(_2, _1)
                ]
            ]
            ;

        start
            =   skip(blank)
            [
                (parse_thread >> "starts" >> parse_thread)
                [
                    _val = phx::construct<StartEvent>(_1, _2)
                ]
            ]
            ;

        join
            =   skip(blank)
            [
                (parse_thread >> "joins" >> parse_thread)
                [
                    _val = phx::construct<JoinEvent>(_1, _2)
                ]
            ]
            ;

        parse_sync_object %= stream_parser<char, SyncObject>();

        parse_thread %= stream_parser<char, Thread>();
    }

private:
    qi::rule<Iterator, Event()> one_event;
    qi::rule<Iterator, AcquireEvent()> acquire;
    qi::rule<Iterator, ReleaseEvent()> release;
    qi::rule<Iterator, StartEvent()> start;
    qi::rule<Iterator, JoinEvent()> join;
    qi::rule<Iterator, SyncObject()> parse_sync_object;
    qi::rule<Iterator, Thread()> parse_thread;

    lock_debug_info_parser<Iterator> info;
};


template <typename Iterator>
struct event_generator : karma::grammar<Iterator, Event()> {

    event_generator() : event_generator::base_type(one_event) {
        using namespace karma;

        one_event = (acquire | release | start | join);

        acquire
            =
            (   stream[_1 = &_val->*&AcquireEvent::thread]
            <<  " acquires "
            <<  stream[_1 = &_val->*&AcquireEvent::lock]
            <<  ' ' << info[_1 = &_val->*&AcquireEvent::info]
            )
            ;

        release
            =   (stream << " releases " << stream)
            [
                _1 = &_val->*&ReleaseEvent::thread,
                _2 = &_val->*&ReleaseEvent::lock
            ]
            ;

        start
            =   (stream << " starts " << stream)
            [
                _1 = &_val->*&StartEvent::parent,
                _2 = &_val->*&StartEvent::child
            ]
            ;

        join
            =   (stream << " joins " << stream)
            [
                _1 = &_val->*&JoinEvent::parent,
                _2 = &_val->*&JoinEvent::child
            ]
            ;
    }

private:
    karma::rule<Iterator, Event()> one_event;
    karma::rule<Iterator, AcquireEvent()> acquire;
    karma::rule<Iterator, ReleaseEvent()> release;
    karma::rule<Iterator, StartEvent()> start;
    karma::rule<Iterator, JoinEvent()> join;

    lock_debug_info_generator<Iterator> info;
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_EVENT_IO_HPP
