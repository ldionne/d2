/**
 * This file defines the `FilesystemLoader` class.
 */

#ifndef D2_FILESYSTEM_LOADER_HPP
#define D2_FILESYSTEM_LOADER_HPP

#include <d2/detail/config.hpp>
#include <d2/events.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/variant.hpp>
#include <vector>


namespace d2 {

/**
 * Class allowing to load the events dispatched by a `FilesystemDispatcher`.
 *
 * This class is not meant to be used by multiple threads at the same time.
 */
class D2_API FilesystemLoader {

    typedef boost::variant<AcquireEvent, ReleaseEvent,
                           StartEvent, JoinEvent, SegmentHopEvent> Event;
    typedef std::vector<Event> unspecified_event_range;

    boost::filesystem::path path_;

    typedef boost::filesystem::ifstream Ifstream;
    static unspecified_event_range load_events(Ifstream& ifs);

public:
    explicit FilesystemLoader(boost::filesystem::path const& path)
                                                            : path_(path) {
        BOOST_ASSERT_MSG(boost::filesystem::is_directory(path),
            "trying to load a path that does not point to a directory");
    }

    /**
     * Call a function on each source of thread events. The function is
     * called with an `unspecified_event_range`.
     */
    template <typename F>
    void for_each(F const& f) {
        namespace fs = boost::filesystem;
        fs::directory_iterator first(path_), last;
        for (; first != last; ++first) {
            fs::path file(*first);
            if (fs::is_regular_file(file)) {
                Ifstream ifs(file);
                BOOST_ASSERT_MSG(ifs,
                    "unable to open the file to load the events");
                f(load_events(ifs));
            }
        }
    }

    /**
     * Return an `unspecified_event_range` containing the events logged at
     * process level.
     */
    inline unspecified_event_range process_events() {
        namespace fs = boost::filesystem;
        fs::path processes(path_);
        processes /= "process_wide";
        BOOST_ASSERT_MSG(fs::is_regular_file(processes),
        "the process_wide file inside the repository is not a regular file");

        Ifstream ifs(processes);
        BOOST_ASSERT_MSG(ifs,
            "unable to open the process_wide file");
        return load_events(ifs);
    }
};

} // end namespace d2

#endif // !D2_FILESYSTEM_LOADER_HPP
