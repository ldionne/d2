/**
 * This file implements the `mock/this_thread.hpp` header.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/mock/this_thread.hpp>
#include <d2/mock/thread.hpp>

#include <boost/thread/thread.hpp>


namespace d2 {
namespace mock {
namespace this_thread {

D2_API extern thread::id get_id() {
    return thread::id(boost::this_thread::get_id());
}

} // end namespace this_thread
} // end namespace mock
} // end namespace d2
