/**
 * This file defines a mock recursive mutex.
 */

#ifndef D2_MOCK_RECURSIVE_MUTEX_HPP
#define D2_MOCK_RECURSIVE_MUTEX_HPP

#include <d2/detail/config.hpp>
#include <d2/mock/mutex.hpp>


namespace d2 {
namespace mock {

struct recursive_mutex : mutex {
    D2_API void lock();

    D2_API void unlock();
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_RECURSIVE_MUTEX_HPP
