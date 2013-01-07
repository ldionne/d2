/**
 * This file defines exceptions dealing with events.
 */

#ifndef D2_EVENTS_EXCEPTIONS_HPP
#define D2_EVENTS_EXCEPTIONS_HPP

#include <d2/events/any_event.hpp>
#include <d2/events/exceptions.hpp>

#include <stdexcept> // for std::runtime_error


namespace d2 {

/**
 * Exception thrown when an event of an unexpected dynamic type is encountered.
 */
struct UnexpectedEventException : virtual std::runtime_error {
    AnyEvent faulty_event; // Optional event that caused the exception.

    explicit UnexpectedEventException(char const* what_arg)
        : std::runtime_error(what_arg)
    { }

    ~UnexpectedEventException() throw() { }
};

} // end namespace d2

#endif // !D2_EVENTS_EXCEPTIONS_HPP
