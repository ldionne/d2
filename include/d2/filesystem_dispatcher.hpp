/**
 * This file defines the `FilesystemDispatcher` class.
 */

#ifndef D2_FILESYSTEM_DISPATCHER_HPP
#define D2_FILESYSTEM_DISPATCHER_HPP

#include <d2/detail/basic_mutex.hpp>
#include <d2/event_traits.hpp>
#include <d2/event_repository.hpp>
#include <d2/thread.hpp>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>


namespace d2 {

/**
 * Class dispatching thread and process level events to a repository.
 *
 * This class is meant to be used concurrently by several threads.
 */
class FilesystemDispatcher {
    // Lock the mapping from thread to stream (and the dummy mapping to the
    // process-wide stream) using a basic_mutex.
    typedef synchronize_with<detail::basic_mutex> EventCategoryLockingPolicy;

    // We lock the access to each stream using a basic_mutex.
    //
    // Locking the process-wide stream is necessary because several
    // threads may need to write to it at the same time.
    //
    // Locking the per-thread streams is also necessary, because threads may
    // emit cross-thread events, i.e. events that go from a thread to another
    // thread's stream (this is currently the case for SegmentHopEvents).
    typedef synchronize_with<detail::basic_mutex> StreamLockingPolicy;

    typedef EventRepository<
                EventCategoryLockingPolicy, StreamLockingPolicy
            > Repository;

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
     * Set a new repository for the event dispatcher. If setting the
     * repository fails, an exception is thrown.
     *
     * @note The method offers the strong exception safety guarantee. If
     *       setting the repository fails, the repository is left unmodified
     *       (as-if the call never happened) and logging continues in the same
     *       repository as before the call.
     */
    template <typename Source>
    void set_repository(Source const& path) {
        // Try to create a new repository.
        // If it throw, the repository won't be modified in any way.
        boost::interprocess::unique_ptr<Repository, RepoDeleter> new_repo;
        new_repo.reset(new Repository(path));

        // "Atomically" exchange the old repository with the new one.
        // This has noexcept guarantee.
        repository_lock_.lock();
        repository_.reset(new_repo.release());
        repository_lock_.unlock();
    }

    /**
     * Same as `set_repository`, but offers the `noexcept` guarantee. Instead
     * of reporting the success or failure using an exception, it will do so
     * using its return value.
     *
     * @return Whether setting a new repository succeeded.
     */
    template <typename Source>
    bool set_repository_noexcept(Source const& path) BOOST_NOEXCEPT {
        try {
            set_repository(path);
        } catch (std::exception const&) {
            return false;
        }
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
            repository->write(Repository::process_wide, event);
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
