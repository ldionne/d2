/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_BASIC_MUTEX_HPP
#define D2_DETAIL_BASIC_MUTEX_HPP

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>

#include <boost/thread/detail/platform.hpp>
#if defined(BOOST_THREAD_PLATFORM_PTHREAD)
#   include <cerrno> // for EINTR
#   include <pthread.h>
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

#error "Win32 not supported right now."

#endif
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_BASIC_MUTEX_HPP
