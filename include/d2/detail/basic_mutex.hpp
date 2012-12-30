/**
 * This file defines several utilities used in the rest of the library.
 * Note: This file contains parts from Boost that were cherry picked and
 *       modified a bit in some cases. Because of this, this file is
 *       distributed under the same license as the original files.
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
#   include <boost/detail/interlocked.hpp>
#   include <boost/thread/win32/interlocked_read.hpp>
#   include <boost/thread/win32/thread_primitives.hpp>
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
    BOOST_STATIC_CONSTANT(unsigned char, lock_flag_bit = 31);
    BOOST_STATIC_CONSTANT(unsigned char, event_set_flag_bit = 30);
    BOOST_STATIC_CONSTANT(long, lock_flag_value = 1 << lock_flag_bit);
    BOOST_STATIC_CONSTANT(long, event_set_flag_value = 1<<event_set_flag_bit);

    long active_count_;
    void* event_;

    void* get_event() {
        void* const current_event =
                            boost::detail::interlocked_read_acquire(&event_);

        if(current_event == NULL) {
            void* const new_event =
            boost::detail::win32::create_anonymous_event(
                ::boost::detail::win32::auto_reset_event,
                ::boost::detail::win32::event_initially_reset);

#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable: 4311 4312)
#endif
            void* const old_event =
            BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER(&event_, new_event, 0);
#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
            if(old_event != NULL) {
                boost::detail::win32::CloseHandle(new_event);
                return old_event;
            } else {
                return new_event;
            }
        }
        return current_event;
    }

    inline void mark_waiting_and_try_lock(long& old_count) {
        while (true) {
            long const new_count = (old_count & lock_flag_value) ?
                            (old_count + 1) : (old_count | lock_flag_value);
            long const current = BOOST_INTERLOCKED_COMPARE_EXCHANGE(
                                        &active_count_, new_count, old_count);
            if(current == old_count)
                break;
            old_count = current;
        }
    }

    inline void clear_waiting_and_try_lock(long& old_count) {
        old_count &= ~lock_flag_value;
        old_count |= event_set_flag_value;
        while (true) {
            long const new_count = (
                (old_count & lock_flag_value) ?
                    old_count :
                    ((old_count - 1) | lock_flag_value)
                ) & ~event_set_flag_value;
            long const current = BOOST_INTERLOCKED_COMPARE_EXCHANGE(
                                        &active_count_, new_count, old_count);
            if(current == old_count)
                break;
            old_count = current;
        }
    }

    inline bool try_lock() BOOST_NOEXCEPT {
        return !boost::detail::win32::interlocked_bit_test_and_set(
                                            &active_count_, lock_flag_bit);
    }

public:
    inline basic_mutex() : active_count_(0), event_(0) { }

    inline ~basic_mutex() {
        void* old_event = BOOST_INTERLOCKED_EXCHANGE_POINTER(&event_, NULL);
        if(old_event)
            boost::detail::win32::CloseHandle(old_event);
    }

    inline void lock() {
        if(try_lock())
            return;

        long old_count = active_count_;
        mark_waiting_and_try_lock(old_count);

        if(old_count & lock_flag_value) {
            bool lock_acquired = false;
            void* const sem = get_event();

            do {
                BOOST_VERIFY(boost::detail::win32::WaitForSingleObject(
                                sem, ::boost::detail::win32::infinite) == 0);
                clear_waiting_and_try_lock(old_count);
                lock_acquired = !(old_count & lock_flag_value);
            } while(!lock_acquired);
        }
    }

    inline void unlock() {
        long const offset = lock_flag_value;
        long const old_count = BOOST_INTERLOCKED_EXCHANGE_ADD(
                                            &active_count_, lock_flag_value);
        if(!(old_count & event_set_flag_value) &&
             old_count > offset &&
            !boost::detail::win32::interlocked_bit_test_and_set(
                                        &active_count_, event_set_flag_bit)) {
            boost::detail::win32::SetEvent(get_event());
        }
    }
#endif // BOOST_THREAD_PLATFORM_{PTHREAD, WIN32}
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_BASIC_MUTEX_HPP
