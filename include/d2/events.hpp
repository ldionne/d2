/**
 * This file defines the event types captured by the logging system.
 */

#ifndef D2_EVENTS_HPP
#define D2_EVENTS_HPP

#include <d2/acquire_event.hpp>
#include <d2/join_event.hpp>
#include <d2/release_event.hpp>
#include <d2/start_event.hpp>

#include <boost/variant.hpp>


namespace d2 {

/**
 * Represents any type of event.
 */
typedef boost::variant<AcquireEvent, ReleaseEvent,
                       StartEvent, JoinEvent> Event;

} // end namespace d2

#endif // !D2_EVENTS_HPP
