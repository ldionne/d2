/**
 * This file defines mutex related utilities.
 */

#ifndef D2_DETAIL_BASIC_MUTEX_HPP
#define D2_DETAIL_BASIC_MUTEX_HPP

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <cstddef> // for NULL

#include <boost/thread/detail/platform.hpp>
#if defined(BOOST_THREAD_PLATFORM_PTHREAD)
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
    basic_mutex() {
        int const res = pthread_mutex_init(&mutex_, NULL);
        BOOST_ASSERT(res == 0); (void)res;
    }

    void lock() {
        int const res = pthread_mutex_lock(&mutex_);
        BOOST_ASSERT(res == 0); (void)res;
    }

    void unlock() {
        int const res = pthread_mutex_unlock(&mutex_);
        BOOST_ASSERT(res == 0); (void)res;
    }

    ~basic_mutex() {
        int const res = pthread_mutex_destroy(&mutex_);
        BOOST_ASSERT(res == 0);(void)res;
    }

#elif defined(BOOST_THREAD_PLATFORM_WIN32)
private:
    CRITICAL_SECTION section_;

public:
    basic_mutex() {
        bool const success =
                ::InitializeCriticalSectionAndSpinCount(&section_, 0) != 0;
        BOOST_ASSERT(success); (void)success;
    }

    void lock() {
        ::EnterCriticalSection(&section_);
    }

    void unlock() {
        ::LeaveCriticalSection(&section_);
    }

    ~basic_mutex() {
        ::DeleteCriticalSection(&section_);
    }
#endif // BOOST_THREAD_PLATFORM_{PTHREAD, WIN32}
};

/**
 * Helper class locking a mutex in its constructor and unlocking it in its
 * destructor.
 */
template <typename Mutex>
class scoped_lock : boost::noncopyable {
    Mutex& mutex_;

public:
    explicit scoped_lock(Mutex& mutex) : mutex_(mutex) {
        mutex_.lock();
    }

    ~scoped_lock() {
        mutex_.unlock();
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_BASIC_MUTEX_HPP
