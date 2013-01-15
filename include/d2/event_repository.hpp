/**
 * This file defines the `EventRepository` class.
 */

#ifndef D2_EVENT_REPOSITORY_HPP
#define D2_EVENT_REPOSITORY_HPP

#include <d2/detail/basic_mutex.hpp>
#include <d2/repository.hpp>
#include <d2/thread.hpp>

#include <boost/mpl/apply.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/utility/value_init.hpp>
#include <string>


namespace d2 {

namespace event_repository_detail {
/**
 * Tag for accessing the process-wide events inside a repository.
 */
struct ProcessWideTag {
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, ProcessWideTag const&)
    { return os << "process_wide", os; }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, ProcessWideTag&) {
        std::string string;
        return is >> string, is;
    }
};

typedef boost::mpl::vector<Thread, ProcessWideTag> EventKeys;

/**
 * Mapping policy to decide what data structure is used to log the different
 * categories of events.
 */
struct EventMapping {
    template <typename Key, typename Stream> struct apply;

    // Each thread has its own sink. Threads are mapped to their sink using
    // a boost::unordered_map.
    template <typename Stream>
    struct apply<Thread, Stream>
        : boost::mpl::apply<boost_unordered_map, Thread, Stream>
    { };

    // Process-wide events are all logged into the same sink. A dummy map
    // mapping all these events to the same sink is used.
    template <typename Stream>
    struct apply<ProcessWideTag, Stream>
        : boost::mpl::apply<unary_map, ProcessWideTag, Stream>
    { };
};

/**
 * Category naming policy. Thread level events go in a subdirectory called
 * "thread_events" and process wide events go in a subdirectory called
 * "process_wide".
 */
struct NamingPolicy {
    template <typename Category>
    static char const* category_path();
};

template <>
char const* NamingPolicy::category_path<Thread>() {
    return "thread_events";
}

template <>
char const* NamingPolicy::category_path<ProcessWideTag>() {
    return "process_wide";
}

} // end namespace event_repository_detail

/**
 * Repository specialized for events.
 *
 * Process wide events are all saved in one file, and per-thread events are
 * saved in a different file for each thread.
 */
template <typename EventCategoryLockingPolicy = no_synchronization,
          typename StreamLockingPolicy = no_synchronization>
struct EventRepository
    : Repository<
        event_repository_detail::EventKeys,
        event_repository_detail::EventMapping,
        EventCategoryLockingPolicy,
        StreamLockingPolicy,
        use_fstream,
        event_repository_detail::NamingPolicy
    >
{
    /**
     * Create a new `EventRepository` using `path` as a directory for storage.
     */
    template <typename Source>
    explicit EventRepository(Source const& path)
        : EventRepository::Repository_(path)
    { }

    /**
     * Special tag for accessing the unique process-wide event stream.
     */
    static event_repository_detail::ProcessWideTag const process_wide;

    /**
     * Type of the stream used for process wide events.
     */
    typedef std::fstream process_wide_stream_type;

    /**
     * Type of a range of thread streams. The type of the range itself is
     * left unspecified, but the type is guaranteed to be a range.
     */
    typedef typename EventRepository::template
                                value_view<Thread>::type thread_stream_range;

    typedef typename EventRepository::template
                    const_value_view<Thread>::type const_thread_stream_range;

    /**
     * Type of the stream used for thread events.
     */
    typedef std::fstream thread_stream_type;

    /**
     * Return a range containing the thread streams.
     */
    thread_stream_range thread_streams() {
        return this->template values<Thread>();
    }

    const_thread_stream_range thread_streams() const {
        return this->template values<Thread>();
    }
};

template <typename EventCategoryLockingPolicy, typename StreamLockingPolicy>
event_repository_detail::ProcessWideTag const
EventRepository<EventCategoryLockingPolicy, StreamLockingPolicy>::
                                    process_wide = boost::initialized_value;

} // end namespace d2

#endif // !D2_EVENT_REPOSITORY_HPP
