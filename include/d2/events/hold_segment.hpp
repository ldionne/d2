/**
 * This file defines the `hold_segment` event.
 */

#ifndef D2_EVENTS_HOLD_SEGMENT_HPP
#define D2_EVENTS_HOLD_SEGMENT_HPP

#include <d2/events/hold_custom.hpp>
#include <d2/segment.hpp>


namespace d2 {

namespace hold_segment_detail {
    struct segment { };
}

/**
 * Class representing an event holding a segment.
 */
D2_DEFINE_CUSTOM_HOLDER(
    hold_segment, hold_segment_detail::segment, Segment, segment_of)

} // end namespace d2

#endif // !D2_EVENTS_HOLD_SEGMENT_HPP
