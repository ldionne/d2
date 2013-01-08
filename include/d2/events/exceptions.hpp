/**
 * This file defines exceptions dealing with events.
 */

#ifndef D2_EVENTS_EXCEPTIONS_HPP
#define D2_EVENTS_EXCEPTIONS_HPP

#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <exception>


namespace d2 {

/**
 * Base class for exceptions related to events. This should be subclassed
 * to give more contextual information.
 */
struct EventException : virtual boost::exception, virtual std::exception { };

namespace exception_tag {
    struct expected_type;
    struct actual_type;
} // end namespace exception_tag

typedef boost::error_info<exception_tag::expected_type, char const*>
                                                                ExpectedType;

typedef boost::error_info<exception_tag::actual_type, char const*> ActualType;

/**
 * Exception thrown when an event of an unexpected dynamic type is encountered.
 */
struct UnexpectedEventException : virtual EventException { };

#define D2_THROW(e) BOOST_THROW_EXCEPTION(e)

} // end namespace d2

#endif // !D2_EVENTS_EXCEPTIONS_HPP
