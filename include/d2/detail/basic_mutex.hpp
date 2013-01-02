/**
 * This file defines several utilities used in the rest of the library.
 * Note: This file is inspired from Boost. Because of this, this file is
 *       distributed under the same license as the original work.
 *
 * (C) Copyright 2006-8 Anthony Williams
 * (C) Copyright 2011-2012 Vicente J. Botet Escriba
 * (C) Copyright 2012 Louis Dionne
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef D2_DETAIL_BASIC_MUTEX_HPP
#define D2_DETAIL_BASIC_MUTEX_HPP

#include <boost/assert.hpp>
#include <boost/config.hpp> // for BOOST_STATIC_CONSTANT
#include <boost/noncopyable.hpp>
#include <cstddef>

#include <boost/thread/detail/platform.hpp>
#if defined(BOOST_THREAD_PLATFORM_PTHREAD)
#   include <cerrno> // for EINTR
#   include <pthread.h>
#elif defined(BOOST_THREAD_PLATFORM_WIN32)
#   include <Windows.h>
#endif


namespace d2 {
namespace detail {

/**
 * Basic mutex class not relying on anything that might be using this library,
 * which would create a circular dependency.
 */
class basic_mutex : public boost::noncopyable {

#if defined(BOOST_THREAD_PLATFORM_PTHREAD)
private:
    pthread_mutex_t mutex_;

public:
    inline basic_mutex() {
        int res = pthread_mutex_init(&mutex_, NULL);
        (void)res;
        BOOST_ASSERT(!res);
    }

    inline void lock() {
        int res;
        do
            res = pthread_mutex_lock(&mutex_);
        while (res == EINTR);
        BOOST_ASSERT(!res);
    }

    inline void unlock() {
        int res;
        do
            res = pthread_mutex_unlock(&mutex_);
        while (res == EINTR);
        BOOST_ASSERT(!res);
    }

    inline ~basic_mutex() {
        int res;
        do
            res = pthread_mutex_destroy(&mutex_);
        while (res == EINTR);
    }

#elif defined(BOOST_THREAD_PLATFORM_WIN32)
private:
    CRITICAL_SECTION section_;

public:
    inline basic_mutex() {
        bool const success =
                        ::InitializeCriticalSectionAndSpinCount(&section_, 0);
        BOOST_ASSERT_MSG(success,
            "failed to initialize the critical section of the basic_mutex");
    }

    inline ~basic_mutex() {
        ::DeleteCriticalSection(&section_);
    }

    inline void lock() {
        ::EnterCriticalSection(&section_);
    }

    inline void unlock() {
        ::LeaveCriticalSection(&section_);
    }
#endif // BOOST_THREAD_PLATFORM_{PTHREAD, WIN32}
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_BASIC_MUTEX_HPP
