/**
 * This file defines exceptions dealing with events.
 */

#ifndef D2_EVENTS_EXCEPTIONS_HPP
#define D2_EVENTS_EXCEPTIONS_HPP

#include <d2/detail/exceptions.hpp>
#include <d2/thread.hpp>


namespace d2 {

/**
 * Base class for exceptions related to events. This should be subclassed
 * to give more contextual information.
 */
struct EventException : virtual Exception {
    virtual char const* what() const throw() {
        return "d2::EventException";
    }
};

/**
 * Exception thrown when an event of an unexpected dynamic type is encountered.
 */
struct EventTypeException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::EventTypeException";
    }
};

namespace exception_tag {
    struct expected_type;
    struct actual_type;
}
typedef boost::error_info<exception_tag::expected_type, char const*>
                                                                ExpectedType;
typedef boost::error_info<exception_tag::actual_type, char const*> ActualType;

} // end namespace d2

#endif // !D2_EVENTS_EXCEPTIONS_HPP
