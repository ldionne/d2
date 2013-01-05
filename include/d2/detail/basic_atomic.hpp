/**
 * This file defines several utilities used in the rest of the library.
 */

#ifndef D2_DETAIL_BASIC_ATOMIC_HPP
#define D2_DETAIL_BASIC_ATOMIC_HPP

#include <d2/detail/basic_mutex.hpp>

#include <boost/noncopyable.hpp>


namespace d2 {
namespace detail {

/**
 * Basic atomic class relying only on `basic_mutex`. We can't use
 * boost/atomic because it relies on locks right now. If boost used
 * this library, we would be in trouble.
 */
template <typename T>
class basic_atomic : public boost::noncopyable {
    T val_;
    basic_mutex mutable lock_;

public:
    basic_atomic(T const& val)
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

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_BASIC_ATOMIC_HPP
