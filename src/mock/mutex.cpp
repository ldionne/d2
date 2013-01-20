/**
 * This file implements the `mock/mutex.hpp` header.
 */

#define D2_SOURCE
#include <d2/api.hpp>
#include <d2/detail/basic_atomic.hpp>
#include <d2/detail/config.hpp>
#include <d2/mock/mutex.hpp>
#include <d2/mock/this_thread.hpp>

#include <cstddef>


namespace d2 {
namespace mock {

namespace {
    detail::basic_atomic<std::size_t> counter(0);
}

mutex::mutex()
    : id_(counter++)
{ }

void mutex::lock() {
    notify_acquire(this_thread::get_id(), *this);
}

void mutex::unlock() {
    notify_release(this_thread::get_id(), *this);
}

D2_API extern std::size_t unique_id(mutex const& self) {
    return self.id_;
}

} // end namespace mock
} // end namespace d2
