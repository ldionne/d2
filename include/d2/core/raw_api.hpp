/*!
 * @file
 * This file defines the raw API to generate events and control `d2`.
 */

#ifndef D2_CORE_RAW_API_HPP
#define D2_CORE_RAW_API_HPP

#include <d2/core/framework_fwd.hpp>
#include <d2/detail/decl.hpp>

#include <boost/config.hpp>
#include <cstddef>
#include <string>


namespace d2 {
namespace core {
namespace raw_api_detail {
    D2_DECL extern framework& get_framework();
}

/**
 * Set the path of the repository into which events are written when logging
 * is enabled.
 *
 * The `path` must either:
 *  - Point to nothing (no file, no directory, etc..).
 *  - Point to an empty directory.
 * Anything else will make the call fail.
 *
 * @return 0 if the operation succeeded, and a non zero value otherwise.
 * @note This operation can be considered atomic.
 * @internal We might associate the return values to error codes in the future.
 */
inline int set_log_repository(char const* path) BOOST_NOEXCEPT {
    return raw_api_detail::get_framework().set_repository(path);
}

inline int set_log_repository(std::string const& path) BOOST_NOEXCEPT {
    return set_log_repository(path.c_str());
}

/**
 * Close the repository into which events are written.
 *
 * @note This operation can be considered atomic.
 * @note Nothing happens if there is no current repository set.
 */
inline void unset_log_repository() BOOST_NOEXCEPT {
    raw_api_detail::get_framework().unset_repository();
}

/**
 * Disable the logging of events by the library.
 *
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already disabled is useless yet harmless.
 */
inline void disable_event_logging() BOOST_NOEXCEPT {
    raw_api_detail::get_framework().disable();
}

/**
 * Enable the logging of events by the library.
 *
 * @note This operation can be considered atomic.
 * @note This function is idempotent, i.e. calling it when the logging is
 *       already enabled is useless yet harmless.
 */
inline void enable_event_logging() BOOST_NOEXCEPT {
    raw_api_detail::get_framework().enable();
}

//! Return whether the logging is currently enabled.
inline bool is_enabled() BOOST_NOEXCEPT {
    return raw_api_detail::get_framework().is_enabled();
}

//! Effectively returns `!is_enabled()`.
inline bool is_disabled() BOOST_NOEXCEPT {
    return raw_api_detail::get_framework().is_disabled();
}

/**
 * Notify the library of the acquisition of a synchronization object with the
 * unique identifier `lock_id` by the thread with the unique identifier
 * `thread_id`.
 */
inline void notify_acquire(std::size_t thread, std::size_t lock) BOOST_NOEXCEPT {
    raw_api_detail::get_framework().notify_acquire(thread, lock);
}

/**
 * Same as `notify_acquire`, but the synchronization object is a
 * recursive synchronization object, i.e. it can be acquired several
 * times by the same thread.
 */
inline void notify_recursive_acquire(std::size_t thread, std::size_t lock) BOOST_NOEXCEPT {
    raw_api_detail::get_framework().notify_recursive_acquire(thread, lock);
}

/**
 * Notify the library of the release of a synchronization object with the
 * unique identifier `lock` by the thread with the unique identifier `thread`.
 */
inline void notify_release(std::size_t thread, std::size_t lock) BOOST_NOEXCEPT {
    raw_api_detail::get_framework().notify_release(thread, lock);
}

/**
 * Same as `notify_release`, but the synchronization object is a
 * recursive synchronization object, i.e. it can be released several
 * times by the same thread.
 */
inline void notify_recursive_release(std::size_t thread, std::size_t lock) BOOST_NOEXCEPT {
    raw_api_detail::get_framework().notify_recursive_release(thread, lock);
}

/**
 * Notify the library of the start of a new thread uniquely identified by
 * `child` created by a thread uniquely identified by `parent`.
 */
inline void notify_start(std::size_t parent, std::size_t child) BOOST_NOEXCEPT {
    raw_api_detail::get_framework().notify_start(parent, child);
}

/**
 * Notify the library of the joining of a thread uniquely identified by
 * `child` into a thread uniquely identified by `parent`.
 */
inline void notify_join(std::size_t parent, std::size_t child) BOOST_NOEXCEPT {
    raw_api_detail::get_framework().notify_join(parent, child);
}
} // end namespace core
} // end namespace d2

#endif // !D2_CORE_RAW_API_HPP
