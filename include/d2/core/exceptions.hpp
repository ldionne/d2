/**
 * This file defines several exception types used in the library.
 */

#ifndef D2_CORE_EXCEPTIONS_HPP
#define D2_CORE_EXCEPTIONS_HPP

#include <d2/core/lock_id.hpp>
#include <d2/core/thread_id.hpp>

#include <boost/exception/all.hpp>
#include <boost/throw_exception.hpp>
#include <exception>


namespace d2 {

#define D2_THROW(e) BOOST_THROW_EXCEPTION(e)

/**
 * Base class for exceptions in the library.
 */
struct Exception : virtual boost::exception, virtual std::exception {
    virtual char const* what() const throw() {
        return "d2::Exception";
    }
};

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

/**
 * Exception thrown when a lock is released and we were not expecting it.
 */
struct UnexpectedReleaseException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::UnexpectedReleaseException";
    }
};

/**
 * Exception thrown when an event comes from an unexpected thread.
 */
struct EventThreadException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::EventThreadException";
    }
};

/**
 * Exception thrown when a recursive lock is locked too many times for the
 * system to handle. While this is _very_ unlikely, we still handle it
 * gracefully.
 */
struct RecursiveLockOverflowException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::RecursiveLockOverflowException";
    }
};

namespace exception_tag {
    struct expected_type;
    struct actual_type;

    struct expected_thread;
    struct actual_thread;

    struct releasing_thread;
    struct released_lock;

    struct overflowing_lock;
    struct current_thread;
} // end namespace exception_tag

typedef boost::error_info<
            exception_tag::expected_type, char const*
        > ExpectedType;

typedef boost::error_info<
            exception_tag::actual_type, char const*
        > ActualType;

typedef boost::error_info<
            exception_tag::expected_thread, ThreadId
        > ExpectedThread;

typedef boost::error_info<
            exception_tag::actual_thread, ThreadId
        > ActualThread;

typedef boost::error_info<
            exception_tag::releasing_thread, ThreadId
        > ReleasingThread;

typedef boost::error_info<
            exception_tag::released_lock, LockId
        > ReleasedLock;

typedef boost::error_info<
            exception_tag::current_thread, ThreadId
        > CurrentThread;

typedef boost::error_info<
            exception_tag::overflowing_lock, LockId
        > OverflowingLock;

} // end namespace d2

#endif // !D2_CORE_EXCEPTIONS_HPP
