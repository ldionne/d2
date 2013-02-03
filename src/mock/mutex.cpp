/**
 * This file implements the `mock/mutex.hpp` header.
 */

#define D2_SOURCE
#include <d2/api.hpp>
#include <d2/detail/config.hpp>
#include <d2/mock/mutex.hpp>
#include <d2/mock/this_thread.hpp>


namespace d2 {
namespace mock {

D2_API void mutex::lock() {
    notify_acquire(this_thread::get_id(), *this);
}

D2_API void mutex::unlock() {
    notify_release(this_thread::get_id(), *this);
}

} // end namespace mock
} // end namespace d2
