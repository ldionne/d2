/**
 * This file defines the `FilesystemDispatcher` class.
 */

#ifndef D2_FILESYSTEM_DISPATCHER_HPP
#define D2_FILESYSTEM_DISPATCHER_HPP

#include <d2/detail/basic_mutex.hpp>
#include <d2/event_traits.hpp>
#include <d2/repository.hpp>
#include <d2/thread.hpp>

#include <boost/assert.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>
#include <string>


namespace d2 {

namespace repository_setup {
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

static ProcessWideTag const process_wide = { };

typedef boost::mpl::vector<Thread, ProcessWideTag> EventKeys;

// Mapping policy for the repository: what is logged where.
struct MappingPolicy {
    template <typename Key, typename Stream> struct apply;

    // Each thread has its own sink. The mapping from Thread to sink uses
    // boost::unordered_map.
    template <typename Stream>
    struct apply<Thread, Stream>
        : boost::mpl::apply<boost_unordered_map, Thread, Stream>
    { };

    // There is another sink; it uses no map at all. It will contain
    // the process-wide events.
    template <typename Stream>
    struct apply<ProcessWideTag, Stream>
        : boost::mpl::apply<unary_map, ProcessWideTag, Stream>
    { };
};

// We lock the access to each stream using a basic_mutex.
//  Locking the process-wide stream is necessary because several
//  threads may need to write to it at the same time.
//  Locking the per-thread streams is also necessary, because threads may
//  emit cross-thread events, i.e. events that go from a thread to
//  another thread's stream (this is currently the case for SegmentHopEvents).
typedef synchronize_with<detail::basic_mutex> StreamLockingPolicy;

// Lock the mapping from thread to stream (and the dummy mapping to the
// process-wide stream) using a basic_mutex.
typedef synchronize_with<detail::basic_mutex> PerKeyLockingPolicy;

// Instantiate the Repository type.
typedef Repository<
            EventKeys, MappingPolicy, PerKeyLockingPolicy, StreamLockingPolicy
        > EventRepository;
} // end namespace repository_setup

/**
 * Class dispatching thread and process level events to a filesystem.
 * Process wide events are all saved in one file, and thread level events
 * are saved in a different file for each thread.
 *
 * This class is meant to be used concurrently by several threads.
 */
class FilesystemDispatcher {
    typedef repository_setup::EventRepository Repository;
    struct RepoDeleter {
        void operator()(Repository* repo) const { delete repo; }
    };

    // We use `shared_ptr` because the `dispatch` methods will be writing into
    // repositories after a call to `set_repository` might have happened into
    // another thread. The `dispatch` methods atomically get the value of the
    // current repository, do their business and then the repository is
    // released automatically if the repository is not referenced anymore
    // because a call to `set_repository` happened.
    boost::shared_ptr<Repository> repository_;
    detail::basic_mutex mutable repository_lock_;

    boost::shared_ptr<Repository> get_repository() {
        boost::shared_ptr<Repository> current_repo;
        repository_lock_.lock();
        current_repo = repository_;
        repository_lock_.unlock();
        return current_repo;
    }

public:
    FilesystemDispatcher() { }

    template <typename Source>
    explicit FilesystemDispatcher(Source const& path)
        : repository_(new Repository(path))
    { }

    /**
     * Set a new repository for the event dispatcher.
     *
     * @return Whether setting a new repository succeeded.
     * @note The method offers the strong exception safety guarantee.
     *       If setting the repository fails, logging will continue in the
     *       same repository as before.
     */
    template <typename Source>
    bool set_repository(Source const& path) {
        // Try to create a new repository.
        boost::interprocess::unique_ptr<Repository, RepoDeleter> new_repo;
        try {
            new_repo.reset(new Repository(path));
        } catch (std::exception const&) {
            return false;
        }

        // "Atomically" exchange the old repository with the new one.
        // This has nothrow guarantee.
        repository_lock_.lock();
        repository_.reset(new_repo.release());
        repository_lock_.unlock();
        return true;
    }

    /**
     * Return whether the `FilesystemDispatcher` currently has a repository
     * in which events are dispatched.
     */
    bool has_repository() const {
        repository_lock_.lock();
        bool const repo_is_not_null = repository_ != 0;
        repository_lock_.unlock();
        return repo_is_not_null;
    }

    template <typename Event>
    typename boost::enable_if<has_event_scope<Event, d2::process_scope>,
    void>::type dispatch(Event const& event) {
        boost::shared_ptr<Repository> repository = get_repository();
        if (repository)
            repository->write(repository_setup::process_wide, event);
    }

    template <typename Event>
    typename boost::enable_if<has_event_scope<Event, d2::thread_scope>,
    void>::type dispatch(Event const& event) {
        boost::shared_ptr<Repository> repository = get_repository();
        if (repository)
            repository->write(thread_of(event), event);
    }
};

} // end namespace d2

#endif // !D2_FILESYSTEM_DISPATCHER_HPP
