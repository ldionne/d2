/**
 * This file implements the `recursive_mutex` class.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/recursive_mutex.hpp>
#include <d2mock/this_thread.hpp>

#include <d2/api.hpp>


namespace d2mock {
namespace recursive_mutex_detail {
D2MOCK_DECL void recursive_mutex::lock() {
    d2::notify_recursive_acquire(this_thread::get_id(), *this);
}

D2MOCK_DECL void recursive_mutex::unlock() {
    d2::notify_recursive_release(this_thread::get_id(), *this);
}
} // end namespace recursive_mutex_detail
} // end namespace d2mock
