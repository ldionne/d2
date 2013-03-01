/**
 * This file defines the `recursive_mutex` class.
 */

#ifndef D2MOCK_RECURSIVE_MUTEX_HPP
#define D2MOCK_RECURSIVE_MUTEX_HPP

#include <d2mock/detail/decl.hpp>
#include <d2mock/mutex.hpp>


namespace d2mock {
namespace recursive_mutex_detail {
struct recursive_mutex : mutex {
    D2MOCK_DECL void lock();

    D2MOCK_DECL void unlock();
};
} // end namespace recursive_mutex_detail

using recursive_mutex_detail::recursive_mutex;
} // end namespace d2mock

#endif // !D2MOCK_RECURSIVE_MUTEX_HPP
