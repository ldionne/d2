/**
 * This file implements the `mock/recursive_mutex.hpp` header.
 */

#define D2_SOURCE
#include <d2/api.hpp>
#include <d2/mock/recursive_mutex.hpp>
#include <d2/mock/this_thread.hpp>


namespace d2 {
namespace mock {

void recursive_mutex::lock() {
    notify_recursive_acquire(this_thread::get_id(), *this);
}

void recursive_mutex::unlock() {
    notify_recursive_release(this_thread::get_id(), *this);
}

} // end namespace mock
} // end namespace d2
