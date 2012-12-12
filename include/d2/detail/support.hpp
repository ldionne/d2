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

#include <d2/btrace/call_stack.hpp>
#include <d2/detail/bounded_io_sequence.hpp>

#include <algorithm>
#include <boost/algorithm/cxx11/copy_n.hpp>
#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/mpl/or.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>


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

struct lock_debug_info {
    std::string file;
    int line;
    typedef std::vector<std::string> CallStack;
    CallStack call_stack;

    inline void init_call_stack(std::size_t max_frames = 100) {
        using namespace boost;
        btrace::dynamic_call_stack cs(max_frames);
        call_stack.reserve(cs.size());
        range::push_back(call_stack, cs | adaptors::transformed(
                        lambda::bind(&btrace::stack_frame::str, lambda::_1)));
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, lock_debug_info const& self) {
        os << make_bounded_output_sequence(self.file)
           << make_bounded_output_sequence(self.file)
           << self.line;
        return os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, lock_debug_info& self) {
        is >> make_bounded_input_sequence(self.file)
           >> make_bounded_input_sequence(self.call_stack)
           >> self.line;
        return is;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_SUPPORT_HPP
