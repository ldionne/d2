/**
 * This file defines the API of the logging framework.
 */

#ifndef D2_LOGGING_HPP
#define D2_LOGGING_HPP

#include <d2/detail/decl.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/move/utility.hpp>


namespace d2 {
namespace logging {
/**
 * Start the framework.
 *
 * Events will be saved in the directory at `path`. If the framework is
 * already `start()`ed, nothing is done. Failure to start the framework
 * due to another problem (such as an internal problem) will be reported
 * by throwing an exception.
 *
 * @note This operation can be considered atomic.
 *
 * @pre `path` points to nothing or points to an empty directory. Failure to
 *      respect the precondition will trigger an assertion.
 *
 * @return Whether the framework was _not_ already started.
 */
D2_DECL extern void start(boost::filesystem::path const& path);

/**
 * Overload of `start` to allow explicit conversions
 * to `boost::filesystem::path`.
 */
template <typename Path>
extern void start(BOOST_FWD_REF(Path) root) {
    start(boost::filesystem::path(boost::forward<Path>(root)));
}

/**
 * Stop the framework.
 *
 * Cleanup is executed (if any), and the framework is back to how it was
 * before calling `start()`. If the framework is not `start()`ed, nothing
 * is done.
 *
 * @note This operation can be considered atomic.
 */
D2_DECL extern void stop();

D2_DECL extern void enable();

/**
 * Temporarily disable the framework.
 *
 * The framework is still up and running, but any event sent to it will be
 * ignored.
 */
D2_DECL extern void disable();

/**
 * Return whether the framework is currently enabled.
 *
 * If the framework has not been started, this function returns `false`.
 */
D2_DECL extern bool is_enabled();

//! Effectively returns `!is_enabled()`.
inline bool is_disabled() {
    return !is_enabled();
}
} // end namespace logging
} // end namespace d2

#endif // !D2_LOGGING_HPP
