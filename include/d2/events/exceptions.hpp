/**
 * This file defines exceptions dealing with events.
 */

#ifndef D2_EVENTS_EXCEPTIONS_HPP
#define D2_EVENTS_EXCEPTIONS_HPP

#include <d2/thread.hpp>

#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <exception>


namespace d2 {

#define D2_THROW(e) BOOST_THROW_EXCEPTION(e)

/**
 * Base class for exceptions related to events. This should be subclassed
 * to give more contextual information.
 */
struct EventException : virtual boost::exception, virtual std::exception { };

/**
 * Exception thrown when an event of an unexpected dynamic type is encountered.
 */
struct EventTypeException : virtual EventException { };

namespace exception_tag {
    struct expected_type;
    struct actual_type;
}
typedef boost::error_info<exception_tag::expected_type, char const*>
                                                                ExpectedType;
typedef boost::error_info<exception_tag::actual_type, char const*> ActualType;

} // end namespace d2

#endif // !D2_EVENTS_EXCEPTIONS_HPP
