/**
 * This file implements the `FilesystemDispatcher` class.
 */

#define D2_SOURCE
#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/config.hpp>
#include <d2/filesystem_dispatcher.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>


namespace d2 {

void FilesystemDispatcher::set_root(boost::filesystem::path const& path) {
    namespace fs = boost::filesystem;
    BOOST_ASSERT_MSG(!fs::exists(path) ||
                    (fs::is_directory(path) && fs::is_empty(path)),
        "the path specified to set_root already exists and is "
        "not an empty directory");

    fs::create_directories(path);
    fs::path processes(path);
    processes /= "process_wide";

    process_sinks_lock_.lock();
    {
        if (process_sinks_.is_open()) {
            process_sinks_.close();
            BOOST_ASSERT_MSG(!process_sinks_.is_open(),
                "failed to close the current process wide event sink");
        }
        process_sinks_.open(processes);
        BOOST_ASSERT_MSG(process_sinks_,
            "unable to open the process wide event sink");
    }
    process_sinks_lock_.unlock();

    // This must happen before clearing the thread_sinks or else
    // a new sink could be created with the old root in between
    // the clear() and the assignment to root_.
    root_lock_.lock();
        root_ = path;
    root_lock_.unlock();

    thread_sinks_lock_.lock();
        thread_sinks_.clear();
    thread_sinks_lock_.unlock();
}

} // end namespace d2
