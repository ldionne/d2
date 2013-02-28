/**
 * This file implements the `d2/uniquely_identifiable.hpp` header.
 */

#define D2_SOURCE
#include <d2/detail/atomic.hpp>
#include <d2/detail/decl.hpp>
#include <d2/uniquely_identifiable.hpp>


namespace d2 {
namespace uniquely_identifiable_detail {
    namespace {
        detail::atomic<unsigned_integral_type> counter(0);
    }

    D2_DECL extern unsigned_integral_type get_unique_id() {
        return counter++;
    }
} // end namespace uniquely_identifiable_detail
} // end namespace d2
