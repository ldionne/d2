/**
 * This file defines the `recursive_mutex` class.
 */

#ifndef D2MOCK_RECURSIVE_MUTEX_HPP
#define D2MOCK_RECURSIVE_MUTEX_HPP

#include <boost/noncopyable.hpp>
#include <d2/basic_lockable.hpp>


namespace d2mock {
namespace recursive_mutex_detail {
struct recursive_mutex : boost::noncopyable {
    void lock() { }

    void unlock() { }
};
} // end namespace recursive_mutex_detail

typedef d2::recursive_basic_lockable<
            recursive_mutex_detail::recursive_mutex
        > recursive_mutex;
} // end namespace d2mock

#endif // !D2MOCK_RECURSIVE_MUTEX_HPP
