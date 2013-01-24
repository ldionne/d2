/**
 * This file defines the `hold_lock_id` event.
 */

#ifndef D2_EVENTS_HOLD_LOCK_ID_HPP
#define D2_EVENTS_HOLD_LOCK_ID_HPP

#include <d2/events/hold_custom.hpp>
#include <d2/lock_id.hpp>


namespace d2 {

namespace hold_lock_id_detail {
    struct lock { };
}

/**
 * Class representing an event that is associated to a lock identifier.
 */
D2_DEFINE_CUSTOM_HOLDER(
    hold_lock_id, hold_lock_id_detail::lock, LockId, lock_of)

} // end namespace d2

#endif // !D2_EVENTS_HOLD_LOCK_ID_HPP
