/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_SUPPORT_HPP
#define D2_DETAIL_SUPPORT_HPP

#include <boost/thread/detail/platform.hpp>
#if defined(BOOST_THREAD_PLATFORM_PTHREAD)
#include <cerrno> // for EINTR
#include <pthread.h>
#endif

#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <cstddef> // for NULL
#include <string>


namespace d2 {
namespace detail {

template <typename T> void remove_unused_variable_warning(T const&) { }

/**
 * Basic mutex class not relying on anything that might be using this library,
 * which would create a circular dependency.
 */
class basic_mutex {
    basic_mutex(basic_mutex const&) /*= delete*/;
    basic_mutex& operator=(basic_mutex const&) /*= delete*/;

#if defined(BOOST_THREAD_PLATFORM_PTHREAD)
private:
    pthread_mutex_t mutex_;

public:
    inline basic_mutex() {
        int res = pthread_mutex_init(&mutex_, NULL);
        remove_unused_variable_warning(res);
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

/**
 * Basic atomic class relying only on `basic_mutex`. We can't use
 * boost/atomic because it relies on locks right now. If boost used
 * this library, we would be in trouble.
 */
template <typename T>
class basic_atomic {
    T val_;
    basic_mutex mutable lock_;

    basic_atomic(basic_atomic const&) /*= delete*/;
    basic_atomic& operator=(basic_atomic const&) /*= delete*/;

public:
    explicit basic_atomic(T const& val)
        : val_(val)
    { }

    basic_atomic& operator=(T const& val) {
        lock_.lock();
        val_ = val;
        lock_.unlock();
        return *this;
    }

    friend T operator++(basic_atomic& self) {
        self.lock_.lock();
        T ret(++self.val_);
        self.lock_.unlock();
        return ret;
    }

    friend T operator++(basic_atomic& self, int) {
        self.lock_.lock();
        T ret(self.val_++);
        self.lock_.unlock();
        return ret;
    }

    friend T operator--(basic_atomic& self) {
        self.lock_.lock();
        T ret(--self.val_);
        self.lock_.unlock();
        return ret;
    }

    friend T operator--(basic_atomic& self, int) {
        self.lock_.lock();
        T ret(self.val_--);
        self.lock_.unlock();
        return ret;
    }

    friend T operator+=(basic_atomic& self, T const& val) {
        self.lock_.lock();
        T ret(self.val_ += val);
        self.lock_.unlock();
        return ret;
    }

    friend T operator-=(basic_atomic& self, T const& val) {
        self.lock_.lock();
        T ret(self.val_ -= val);
        self.lock_.unlock();
        return ret;
    }

    operator T() const {
        lock_.lock();
        T ret(val_);
        lock_.unlock();
        return ret;
    }
};

/**
 * Simple wrapper over a string that modifies its << and >> operators to
 * make sure the string is saved/loaded as-is.
 */
template <typename String>
struct delimited_type {
    String s_;
    explicit delimited_type(String s) : s_(s) { }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, delimited_type self) {
        os << self.s_.size() << ':' << self.s_;
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, delimited_type self) {
        typename boost::remove_reference<String>::type::size_type size;
        char colon;
        is >> size >> colon;
        self.s_.reserve(size);
        while (size--)
            self.s_.push_back(is.get());
        return is;
    }
};

inline delimited_type<std::string const&> delimited(std::string const& s){
    return delimited_type<std::string const&>(s);
}

inline delimited_type<std::string&> delimited(std::string& s) {
    return delimited_type<std::string&>(s);
}

struct lock_debug_info {
    std::string file;
    int line;

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, lock_debug_info const& self) {
        return os << delimited(self.file) << self.line, os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, lock_debug_info& self) {
        return is >> delimited(self.file) >> self.line, is;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_SUPPORT_HPP
