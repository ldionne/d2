/**
 * This file defines the `FilesystemDispatcher` class.
 */

#ifndef D2_CORE_FILESYSTEM_DISPATCHER_HPP
#define D2_CORE_FILESYSTEM_DISPATCHER_HPP

#include <d2/core/filesystem.hpp>
#include <d2/detail/mutex.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/config.hpp>
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/move/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <dyno/serializing_stream.hpp>
#include <fstream>
#include <ios>
#include <string>


namespace d2 {
namespace filesystem_dispatcher_detail {

typedef core::filesystem<
            dyno::serializing_stream<
                std::ofstream,
                boost::archive::text_oarchive
            >
        > Filesystem;

/**
 * Class dispatching thread and process level events to a repository.
 *
 * This class is meant to be used concurrently by several threads.
 */
class FilesystemDispatcher {
    struct default_deleter {
        template <typename T>
        void operator()(T* ptr) const { delete ptr; }
    };

    // We use `shared_ptr` because the `dispatch` methods will be writing into
    // repositories after a call to `set_repository` might have happened into
    // another thread. The `dispatch` method atomically get the value of the
    // current repository, do its business and then the repository is
    // released automatically if the repository is not referenced anymore
    // because a call to `set_repository` happened.
    boost::shared_ptr<Filesystem> repository_;
    detail::mutex mutable repository_lock_;
    typedef boost::lock_guard<detail::mutex> scoped_lock;

    boost::shared_ptr<Filesystem> get_repository() {
        scoped_lock lock(repository_lock_);
        return repository_;
    }

public:
    FilesystemDispatcher()
        : repository_()
    { }

    template <typename Path>
    explicit FilesystemDispatcher(BOOST_FWD_REF(Path) root)
        : repository_(boost::make_shared<Filesystem>(
                                        root, std::ios::out | std::ios::ate))
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
    template <typename Path>
    void set_repository(BOOST_FWD_REF(Path) new_root) {
        // Try to create a new repository.
        // If it throws, the repository won't be modified in any way.
        typedef boost::interprocess::unique_ptr<
                    Filesystem, default_deleter
                > FilesystemPtr;

        FilesystemPtr new_fs(new Filesystem(
            boost::forward<Path>(new_root), std::ios::out | std::ios::ate));

        // "Atomically" exchange the old repository with the new one.
        // This has noexcept guarantee.
        {
            scoped_lock lock(repository_lock_);
            repository_.reset(new_fs.release());
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
    template <typename Path>
    bool set_repository_noexcept(BOOST_FWD_REF(Path) root) BOOST_NOEXCEPT {
        try {
            set_repository(boost::forward<Path>(root));
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
    void dispatch(BOOST_FWD_REF(Event) event) {
        boost::shared_ptr<Filesystem> repository = get_repository();
        if (repository)
            repository->dispatch(boost::forward<Event>(event));
    }
};
} // end namespace filesystem_dispatcher_detail

namespace core {
    using filesystem_dispatcher_detail::FilesystemDispatcher;
}
} // end namespace d2

#endif // !D2_CORE_FILESYSTEM_DISPATCHER_HPP
