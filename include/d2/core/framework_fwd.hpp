/**
 * This file forward declares `d2::core::framework`.
 */

#ifndef D2_CORE_FRAMEWORK_FWD_HPP
#define D2_CORE_FRAMEWORK_FWD_HPP

#include <d2/core/filesystem_dispatcher.hpp>
#include <d2/core/segment.hpp>
#include <d2/core/thread_id.hpp>
#include <d2/detail/atomic.hpp>
#include <d2/detail/mutex.hpp>

#include <boost/unordered_map.hpp>
#include <cstddef>


namespace d2 {
namespace core {
class framework {
public:
    framework();
    ~framework();

    void enable();
    void disable();

    bool is_enabled() const;
    bool is_disabled() const;

    int set_repository(char const* path);
    void unset_repository();

    void notify_acquire(std::size_t thread, std::size_t lock);
    void notify_recursive_acquire(std::size_t thread, std::size_t lock);

    void notify_release(std::size_t thread, std::size_t lock);
    void notify_recursive_release(std::size_t thread, std::size_t lock);

    void notify_start(std::size_t parent, std::size_t child);
    void notify_join(std::size_t parent, std::size_t child);

private:
    FilesystemDispatcher dispatcher_;
    detail::atomic<bool> event_logging_enabled_;

    // default initialized to the initial segment value
    Segment current_segment;
    detail::mutex segment_mutex;
    boost::unordered_map<ThreadId, Segment> segment_of;
};
} // end namespace core
} // end namespace d2

#endif // !D2_CORE_FRAMEWORK_FWD_HPP
