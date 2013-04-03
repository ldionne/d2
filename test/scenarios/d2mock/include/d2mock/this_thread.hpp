/**
 * This file defines facilities to access the current thread.
 */

#ifndef D2MOCK_THIS_THREAD_HPP
#define D2MOCK_THIS_THREAD_HPP

#include <d2mock/detail/decl.hpp>
#include <d2mock/thread.hpp>


namespace d2mock {
namespace this_thread {
    //! Return the thread id of the current thread.
    D2MOCK_DECL extern thread::id get_id();
} // end namespace this_thread
} // end namespace d2mock

#endif // !D2MOCK_THIS_THREAD_HPP
