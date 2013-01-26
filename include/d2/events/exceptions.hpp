/**
 * This file defines exceptions dealing with events.
 */

#ifndef D2_EVENTS_EXCEPTIONS_HPP
#define D2_EVENTS_EXCEPTIONS_HPP

#include <d2/detail/exceptions.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>


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

#endif // !D2_EVENTS_EXCEPTIONS_HPP
