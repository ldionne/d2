/**
 * This file defines exceptions dealing with events.
 */

#ifndef D2_EVENTS_EXCEPTIONS_HPP
#define D2_EVENTS_EXCEPTIONS_HPP

#include <boost/spirit/home/support/detail/hold_any.hpp>
#include <stdexcept> // for std::runtime_error


namespace d2 {

/**
 * Exception thrown when an event of an unexpected dynamic type is encountered.
 */
struct UnexpectedEventException : virtual std::runtime_error {
    // Optional event that caused the exception.
    boost::spirit::hold_any faulty_event;

    explicit UnexpectedEventException(char const* what_arg)
        : std::runtime_error(what_arg)
    { }

    ~UnexpectedEventException() throw() { }
};

} // end namespace d2

#endif // !D2_EVENTS_EXCEPTIONS_HPP
