/**
 * This file defines the `atomic` class.
 */

#ifndef D2_DETAIL_ATOMIC_HPP
#define D2_DETAIL_ATOMIC_HPP

#include <d2/detail/mutex.hpp>

#include <boost/noncopyable.hpp>


namespace d2 {
namespace detail {
/**
 * Basic atomic class relying only on `mutex`. We can't use `Boost.Atomic`
 * it relies on locks right now. If Boost used this library, we would be
 * in trouble.
 */
template <typename T>
class atomic : public boost::noncopyable {
    T val_;
    mutex mutable mutex_;

public:
    // val_ voluntarily left uninitialized. atomic should have
    // the same behavior as T, but with the operators being atomic.
    atomic() { }

    atomic(T const& val)
        : val_(val)
    { }

    atomic& operator=(T const& val) {
        scoped_lock<mutex> lock(mutex_);
        val_ = val;
        return *this;
    }

    friend T operator++(atomic& self) {
        scoped_lock<mutex> lock(self.mutex_);
        T ret(++self.val_);
        return ret;
    }

    friend T operator++(atomic& self, int) {
        scoped_lock<mutex> lock(self.mutex_);
        T ret(self.val_++);
        return ret;
    }

    friend T operator--(atomic& self) {
        scoped_lock<mutex> lock(self.mutex_);
        T ret(--self.val_);
        return ret;
    }

    friend T operator--(atomic& self, int) {
        scoped_lock<mutex> lock(self.mutex_);
        T ret(self.val_--);
        return ret;
    }

    friend T operator+=(atomic& self, T const& val) {
        scoped_lock<mutex> lock(self.mutex_);
        T ret(self.val_ += val);
        return ret;
    }

    friend T operator-=(atomic& self, T const& val) {
        scoped_lock<mutex> lock(self.mutex_);
        T ret(self.val_ -= val);
        return ret;
    }

    operator T() const {
        scoped_lock<mutex> lock(mutex_);
        T ret(val_);
        return ret;
    }
};
} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_ATOMIC_HPP
