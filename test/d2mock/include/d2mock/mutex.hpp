/**
 * This file defines the `mutex` class.
 */

#ifndef D2MOCK_MUTEX_HPP
#define D2MOCK_MUTEX_HPP

#include <boost/noncopyable.hpp>
#include <d2/basic_lockable.hpp>


namespace d2mock {
namespace mutex_detail {
struct mutex : boost::noncopyable {
    void lock() { }

    void unlock() { }
};
} // end namespace mutex_detail

typedef d2::basic_lockable<mutex_detail::mutex, false> mutex;
} // end namespace d2

#endif // !D2MOCK_MUTEX_HPP
