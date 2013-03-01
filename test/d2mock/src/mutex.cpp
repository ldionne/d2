/**
 * This file implements the `mutex` class.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/mutex.hpp>
#include <d2mock/this_thread.hpp>

#include <d2/api.hpp>


namespace d2mock {
namespace mutex_detail {
D2MOCK_DECL void mutex::lock() {
    d2::notify_acquire(this_thread::get_id(), *this);
}

D2MOCK_DECL void mutex::unlock() {
    d2::notify_release(this_thread::get_id(), *this);
}
} // end namespace mutex_detail
} // end namespace d2mock
