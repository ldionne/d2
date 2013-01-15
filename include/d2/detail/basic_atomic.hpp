/**
 * This file defines a pseudo-atomic class.
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
    basic_mutex mutable mutex_;

public:
    // val_ voluntarily left uninitialized. basic_atomic should have
    // the same behavior as T, but with the operators being atomic.
    basic_atomic() { }

    basic_atomic(T const& val)
        : val_(val)
    { }

    basic_atomic& operator=(T const& val) {
        scoped_lock<basic_mutex> lock(mutex_);
        val_ = val;
        return *this;
    }

    friend T operator++(basic_atomic& self) {
        scoped_lock<basic_mutex> lock(self.mutex_);
        T ret(++self.val_);
        return ret;
    }

    friend T operator++(basic_atomic& self, int) {
        scoped_lock<basic_mutex> lock(self.mutex_);
        T ret(self.val_++);
        return ret;
    }

    friend T operator--(basic_atomic& self) {
        scoped_lock<basic_mutex> lock(self.mutex_);
        T ret(--self.val_);
        return ret;
    }

    friend T operator--(basic_atomic& self, int) {
        scoped_lock<basic_mutex> lock(self.mutex_);
        T ret(self.val_--);
        return ret;
    }

    friend T operator+=(basic_atomic& self, T const& val) {
        scoped_lock<basic_mutex> lock(self.mutex_);
        T ret(self.val_ += val);
        return ret;
    }

    friend T operator-=(basic_atomic& self, T const& val) {
        scoped_lock<basic_mutex> lock(self.mutex_);
        T ret(self.val_ -= val);
        return ret;
    }

    operator T() const {
        scoped_lock<basic_mutex> lock(mutex_);
        T ret(val_);
        return ret;
    }
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_BASIC_ATOMIC_HPP
