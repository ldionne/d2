/**
 * This file implements the `this_thread` namespace.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/this_thread.hpp>
#include <d2mock/thread.hpp>

#include <boost/thread/thread.hpp>


namespace d2mock {
namespace this_thread {
    D2MOCK_DECL extern thread::id get_id() {
        return thread::id(boost::this_thread::get_id());
    }
} // end namespace this_thread
} // end namespace d2mock
