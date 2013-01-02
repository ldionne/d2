/**
 * This file defines means to perform input/output of events.
 */

#ifndef D2_DETAIL_EVENT_IO_HPP
#define D2_DETAIL_EVENT_IO_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/events.hpp>
#include <d2/types.hpp>

#include <boost/phoenix.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <string>


namespace d2 {
namespace detail {

namespace qi = boost::spirit::qi;
namespace karma = boost::spirit::karma;

template <typename Iterator>
struct lock_debug_info_parser : qi::grammar<Iterator, LockDebugInfo()> {

    lock_debug_info_parser() : lock_debug_info_parser::base_type(start) {
        using namespace qi;

        start = -(call_stack[&_val->*&LockDebugInfo::call_stack = qi::_1]);

        call_stack %= "[[" >> *stack_frame >> "]]";

        stack_frame
            %=  as<void*>()[voidp] >> omit[blank]
            >>  +(char_ - "%%") >> "%%"
            >>  +~char_('\n') >> '\n'
            ;
    }

private:
    qi::rule<Iterator, LockDebugInfo()> start;
    qi::rule<Iterator, LockDebugInfo::CallStack()> call_stack;
    qi::rule<Iterator, StackFrame()> stack_frame;
    qi::stream_parser<char, void*> voidp;
};

template <typename Iterator>
struct lock_debug_info_generator : karma::grammar<Iterator,LockDebugInfo()>{

    lock_debug_info_generator() : lock_debug_info_generator::base_type(start){
        using namespace karma;
        namespace phx = boost::phoenix;
        using phx::arg_names::arg1;

        start
            =   (-call_stack)
            [
                phx::if_(!phx::bind(&LockDebugInfo::CallStack::empty,
                                    &_val->*&LockDebugInfo::call_stack))[
                    arg1 = &_val->*&LockDebugInfo::call_stack
                ]
            ]
            ;

        call_stack %= "[[" << *stack_frame << "]]";

        stack_frame %= hex << ' ' << +char_ << "%%" << +char_ << '\n';
    }

private:
    karma::rule<Iterator, LockDebugInfo()> start;
    karma::rule<Iterator, LockDebugInfo::CallStack()> call_stack;
    karma::rule<Iterator, StackFrame()> stack_frame;
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
                    _val = phx::construct<AcquireEvent>(qi::_2, qi::_1)
                ]
                >> -(info[&_val->*&AcquireEvent::info = qi::_1])
            ]
            ;

        release
            =   skip(blank)
            [
                (parse_thread >> "releases" >> parse_sync_object)
                [
                    _val = phx::construct<ReleaseEvent>(qi::_2, qi::_1)
                ]
            ]
            ;

        start
            =   skip(blank)
            [
                (parse_thread >> "starts" >> parse_thread)
                [
                    _val = phx::construct<StartEvent>(qi::_1, qi::_2)
                ]
            ]
            ;

        join
            =   skip(blank)
            [
                (parse_thread >> "joins" >> parse_thread)
                [
                    _val = phx::construct<JoinEvent>(qi::_1, qi::_2)
                ]
            ]
            ;

        // Note: stream_parser is currently broken in spirit, so this is a temporary solution.
        parse_sync_object = ulong_[_val = phx::construct<SyncObject>(qi::_1)];//stream_parser<char, SyncObject>();

        parse_thread = ulong_[_val = phx::construct<Thread>(qi::_1)];//stream_parser<char, Thread>();
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
            (   stream[qi::_1 = &_val->*&AcquireEvent::thread]
            <<  " acquires "
            <<  stream[qi::_1 = &_val->*&AcquireEvent::lock]
            <<  ' ' << info[qi::_1 = &_val->*&AcquireEvent::info]
            )
            ;

        release
            =   (stream << " releases " << stream)
            [
                qi::_1 = &_val->*&ReleaseEvent::thread,
                qi::_2 = &_val->*&ReleaseEvent::lock
            ]
            ;

        start
            =   (stream << " starts " << stream)
            [
                qi::_1 = &_val->*&StartEvent::parent,
                qi::_2 = &_val->*&StartEvent::child
            ]
            ;

        join
            =   (stream << " joins " << stream)
            [
                qi::_1 = &_val->*&JoinEvent::parent,
                qi::_2 = &_val->*&JoinEvent::child
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
