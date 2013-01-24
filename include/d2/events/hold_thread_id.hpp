/**
 * This file defines the `hold_thread_id` event.
 */

#ifndef D2_EVENTS_HOLD_THREAD_ID_HPP
#define D2_EVENTS_HOLD_THREAD_ID_HPP

#include <d2/events/hold_custom.hpp>
#include <d2/thread_id.hpp>


namespace d2 {

namespace hold_thread_id_detail {
    struct thread { };
}

/**
 * Class representing an event that is aware of the thread in which it was
 * generated.
 */
D2_DEFINE_CUSTOM_HOLDER(
    hold_thread_id, hold_thread_id_detail::thread, ThreadId, thread_of)

} // end namespace d2

#endif // !D2_EVENTS_HOLD_THREAD_ID_HPP
