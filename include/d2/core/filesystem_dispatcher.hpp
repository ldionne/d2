/**
 * This file defines the `FilesystemDispatcher` class.
 */

#ifndef D2_CORE_FILESYSTEM_DISPATCHER_HPP
#define D2_CORE_FILESYSTEM_DISPATCHER_HPP

#include <d2/detail/mutex.hpp>
#include <d2/event_traits.hpp>

#include <boost/config.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>
#include <dyno/filesystem.hpp>
#include <fstream>
#include <string>


namespace d2 {
namespace filesystem_dispatcher_detail {

struct EventMappingPolicy {
    template <typename Event>
    typename boost::enable_if<has_event_scope<Event, thread_scope>,
    std::string>::type operator()(Event const& event) const {
        return boost::lexical_cast<std::string>(thread_of(event));
    }

    template <typename Event>
    typename boost::enable_if<has_event_scope<Event, process_scope>,
    std::string>::type operator()(Event const&) const {
        return "process_wide";
    }
};

typedef dyno::filesystem<EventMappingPolicy, std::fstream> Repository;

/**
 * Class dispatching thread and process level events to a repository.
 *
 * This class is meant to be used concurrently by several threads.
 */
class FilesystemDispatcher {
    struct RepoDeleter {
        void operator()(Repository* repo) const { delete repo; }
    };

    // We use `shared_ptr` because the `dispatch` methods will be writing into
    // repositories after a call to `set_repository` might have happened into
    // another thread. The `dispatch` method atomically get the value of the
    // current repository, do its business and then the repository is
    // released automatically if the repository is not referenced anymore
    // because a call to `set_repository` happened.
    boost::shared_ptr<Repository> repository_;
    detail::mutex mutable repository_lock_;
    typedef detail::scoped_lock<detail::mutex> scoped_lock;

    boost::shared_ptr<Repository> get_repository() {
        scoped_lock lock(repository_lock_);
        return repository_;
    }

public:
    FilesystemDispatcher()
        : repository_()
    { }

    template <typename Source>
    explicit FilesystemDispatcher(Source const& path)
        : repository_(boost::make_shared<Repository>(path))
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
        // If it throws, the repository won't be modified in any way.
        namespace ipc = boost::interprocess;
        ipc::unique_ptr<Repository,RepoDeleter> new_repo(new Repository(path));

        // "Atomically" exchange the old repository with the new one.
        // This has noexcept guarantee.
        {
            scoped_lock lock(repository_lock_);
            repository_.reset(new_repo.release());
        }
    }

    /**
     * Unset the current repository.
     */
    void unset_repository() {
        scoped_lock lock(repository_lock_);
        repository_.reset();
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
        scoped_lock lock(repository_lock_);
        return static_cast<bool>(repository_);
    }

    template <typename Event>
    void dispatch(Event const& event) {
        boost::shared_ptr<Repository> repository = get_repository();
        if (repository)
            repository->dispatch(event);
    }
};
} // end namespace filesystem_dispatcher_detail

namespace core {
    using filesystem_dispatcher_detail::FilesystemDispatcher;
    typedef filesystem_dispatcher_detail::Repository Filesystem;
}
} // end namespace d2

#endif // !D2_CORE_FILESYSTEM_DISPATCHER_HPP
