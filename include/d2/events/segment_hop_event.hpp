/**
 * This file defines the `SegmentHopEvent` event.
 */

#ifndef D2_EVENTS_SEGMENT_HOP_EVENT_HPP
#define D2_EVENTS_SEGMENT_HOP_EVENT_HPP

#include <d2/detail/config.hpp>
#include <d2/event_traits.hpp>
#include <d2/events/hold_custom.hpp>
#include <d2/segment.hpp>
#include <d2/thread_id.hpp>

#include <iosfwd>


namespace d2 {

namespace segment_hop_event_detail {
    struct segment { };
    struct thread { };
    D2_DEFINE_CUSTOM_HOLDER(hold_segment, segment, Segment, segment_of)
    D2_DEFINE_CUSTOM_HOLDER(hold_thread, thread, ThreadId, thread_of)
}

/**
 * Represents the entrance of a thread into a new segment. This happens when
 * a thread starts a child thread or when a thread is a child thread itself.
 */
struct SegmentHopEvent
    : segment_hop_event_detail::hold_segment<
        segment_hop_event_detail::hold_thread<>
    >
{
    /**
     * This constructor must only be used when serializing events.
     * The object is in an invalid state once default-constructed.
     */
    SegmentHopEvent() { }

    SegmentHopEvent(ThreadId const& thread, Segment const& segment) {
        thread_of(*this) = thread;
        segment_of(*this) = segment;
    }

    D2_API friend std::istream& operator>>(std::istream&, SegmentHopEvent&);
    D2_API friend
    std::ostream& operator<<(std::ostream&, SegmentHopEvent const&);

    typedef thread_scope event_scope;
    typedef strict_order_policy ordering_policy;
};

} // end namespace d2

#endif // !D2_EVENTS_SEGMENT_HOP_EVENT_HPP
