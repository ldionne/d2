/**
 * This file defines facilities to access the current thread.
 */

#ifndef D2_MOCK_THIS_THREAD_HPP
#define D2_MOCK_THIS_THREAD_HPP

#include <d2/detail/config.hpp>
#include <d2/mock/thread.hpp>


namespace d2 {
namespace mock {
namespace this_thread {

/**
 * Return the thread id of the current thread.
 */
D2_API extern thread::id get_id();

} // end namespace this_thread
} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_THIS_THREAD_HPP
