/**
 * This file defines the `FilesystemDispatcher` class.
 */

#ifndef D2_FILESYSTEM_DISPATCHER_HPP
#define D2_FILESYSTEM_DISPATCHER_HPP

#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/config.hpp>
#include <d2/event_traits.hpp>
#include <d2/thread.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/enable_if.hpp>
#include <string>


namespace d2 {

/**
 * Class dispatching thread and process level events to a filesystem.
 * Process wide events are all saved in one file, and thread level events
 * are saved in a different file for each thread.
 */
class D2_API FilesystemDispatcher {
    typedef boost::filesystem::ofstream Ofstream;

    detail::basic_mutex mutable process_sinks_lock_;
    Ofstream process_sinks_;

    typedef boost::unordered_map<Thread, Ofstream> ThreadSinks;
    ThreadSinks thread_sinks_;
    detail::basic_mutex mutable thread_sinks_lock_;

    detail::basic_mutex mutable root_lock_;
    boost::filesystem::path root_;

    inline boost::filesystem::path path_for(Thread const& thread) const {
        namespace fs = boost::filesystem;
        root_lock_.lock();
            fs::path path(root_);
        root_lock_.unlock();
        path /= "thread_" + boost::lexical_cast<std::string>(thread);
        return path;
    }

public:
    inline FilesystemDispatcher() { }

    inline explicit FilesystemDispatcher(boost::filesystem::path const& path){
        set_root(path);
    }

    void set_root(boost::filesystem::path const& path);

    inline bool has_root() const {
        root_lock_.lock();
            bool const empty = root_.empty();
        root_lock_.unlock();
        return !empty;
    }

    template <typename Event>
    typename boost::enable_if<has_event_scope<Event, d2::process_scope>,
    void>::type dispatch(Event const& event) {
        BOOST_ASSERT_MSG(has_root(),
            "trying to dispatch an event in a dispatcher not assigned "
            "to a repository");
        process_sinks_lock_.lock();
            process_sinks_ << event << '\n';
        process_sinks_lock_.unlock();
    }

    template <typename Event>
    typename boost::enable_if<has_event_scope<Event, d2::thread_scope>,
    void>::type dispatch(Event const& event) {
        BOOST_ASSERT_MSG(has_root(),
            "trying to dispatch an event in a dispatcher not assigned "
            "to a repository");
        Thread const& thread = thread_of(event);

        thread_sinks_lock_.lock();
        {
            Ofstream& sink = thread_sinks_[thread];
            if (!sink.is_open()) {
                sink.open(path_for(thread));
                BOOST_ASSERT_MSG(sink, "unable to open a thread's sink");
            }
            sink << event << '\n';
        }
        thread_sinks_lock_.unlock();
    }
};

} // end namespace d2

#endif // !D2_FILESYSTEM_DISPATCHER_HPP
