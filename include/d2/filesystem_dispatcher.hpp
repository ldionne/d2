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
#include <string>


namespace d2 {

/**
 * Class dispatching thread and process level events to a filesystem.
 * Process wide events are all saved in one file, and thread level events
 * are saved in a different file for each thread.
 */
class D2_API FilesystemDispatcher {
    typedef boost::filesystem::ofstream Ofstream;

    detail::basic_mutex process_lock_;
    Ofstream process_sinks_;

    typedef boost::unordered_map<Thread, Ofstream> ThreadSinks;
    ThreadSinks thread_sinks_;
    detail::basic_mutex thread_lookup_lock_;

    boost::filesystem::path root_;

    inline void initialize() {
        namespace fs = boost::filesystem;
        BOOST_ASSERT_MSG(!fs::exists(root_) ||
                        (fs::is_directory(root_) && fs::is_empty(root_)),
            "the path specified as the root of the FilesystemDispatcher "
            "already exists and is not an empty directory");

        fs::create_directories(root_);
        fs::path processes(root_);
        process_sinks_.open(processes /= "process_wide");

        BOOST_ASSERT_MSG(process_sinks_,
            "unable to open the process wide event sink");
    }

    inline boost::filesystem::path path_for(Thread const& thread) const {
        namespace fs = boost::filesystem;
        fs::path path(root_);
        path /= "thread_" + boost::lexical_cast<std::string>(thread);
        return path;
    }

public:
    inline explicit FilesystemDispatcher(boost::filesystem::path const& root)
        : root_(root)
    { initialize(); }

    template <typename Event>
    void dispatch(Event const& event, d2::process_scope) {
        process_lock_.lock();
        process_sinks_ << event;
        process_lock_.unlock();
    }

    template <typename Event>
    void dispatch(Event const& event, d2::thread_scope) {
        Thread const& thread = thread_of(event);

        thread_lookup_lock_.lock();
        Ofstream& sink = thread_sinks_[thread];
        thread_lookup_lock_.unlock();

        if (!sink.is_open()) {
            sink.open(path_for(thread));
            BOOST_ASSERT_MSG(sink, "unable to open a thread's sink");
        }
        sink << event;
    }
};

#if 0
namespace detail {
struct FileSystemDispatcherPolicy {
    // template <typename Scope>
    // struct apply {
    //     typedef boost::mpl::vector<
    //                 boost::mpl::pair<d2::global_scope, std::ofstream>,
    //                 boost::mpl::pair<d2::machine_scope, std::ofstream>,
    //                 boost::mpl::pair<d2::process_scope, std::map<some_process_id, std::ofstream> >,
    //                 boost::mpl::pair<d2::thread_scope, std::map<some_thread_id, std::ofstream> >
    //             > SinkTypes;
    //     typedef typename boost::mpl::find_if<
    //                 SinkTypes,
    //                 boost::is_same<Scope, boost::mpl::first<boost::mpl::_1> >
    //             >::type Iter;
    //     typedef typename boost::mpl::deref<Iter>::type Pair;
    //     typedef typename boost::mpl::second<Pair>::type type;
    // };

    template <typename Scope, typename Dummy = void>
    struct apply;

    template <typename Dummy>
    struct apply<d2::process_scope, Dummy> {
        typedef std::ofstream type;
    };

    template <typename Dummy>
    struct apply<d2::thread_scope, Dummy> {
        typedef boost::unordered_map<Thread, std::ofstream> type;
    };

    template <typename Event>
    static std::ostream& get_sink(std::ofstream& sink, Event const&, d2::process_scope) {
        assert(!sink.open()); // how can we initialize it?
        return sink;
    }

    template <typename Sinks, typename Event>
    static std::ostream& get_sink(Sinks& sinks, Event const& event, d2::thread_scope) {
        Thread thread = thread_of(event);
        sinks.lock.lock();
        typename Sinks::Map::iterator found = sinks.map.find(thread);
        sinks.lock.unlock();

        if (sinks.find(thread) == sinks.end()) {
            sinks.insert(std::make_pair(thread, std::ofstream(thread.to_string())));
        }
        return sinks[thread];
    }
};
} // end namespace detail

/**
 * Event dispatcher storing the events on a filesystem.
 * Layout of the filesystem is as follow:
 *  - For each scoping level, one file containing all the associated events
 *    at that scoping level.
 *  - One directory containing all the associated finer grained events.
 */
template <typename Scopes = use_default>
struct FilesystemDispatcher
    : EventDispatcher<detail::FileSystemDispatcherPolicy, Scopes>
{ };
#endif

} // end namespace d2

#endif // !D2_FILESYSTEM_DISPATCHER_HPP
