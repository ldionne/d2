/*!
 * @file
 * This file contains unit tests for the `d2::trackable_thread` class.
 */

#include <d2/trackable_thread.hpp>

#include <boost/move/move.hpp>


namespace {
/*!
 * @internal
 * Dummy class with a standard thread interface for unit testing purpose.
 */
struct untracked_thread {
    class id { };
    typedef void native_handle_type;

    untracked_thread() BOOST_NOEXCEPT { }
    template <typename F>
    explicit untracked_thread(BOOST_FWD_REF(F), ...) { }
    ~untracked_thread() { }

    untracked_thread(BOOST_RV_REF(untracked_thread)) BOOST_NOEXCEPT { }

    untracked_thread&
    operator=(BOOST_RV_REF(untracked_thread)) BOOST_NOEXCEPT { return *this; }

    void swap(untracked_thread&) BOOST_NOEXCEPT { }
    friend void
    swap(untracked_thread& self, untracked_thread& other) BOOST_NOEXCEPT { }

    bool joinable() const BOOST_NOEXCEPT { return false; }
    void join() { }
    void detach() { }
    id get_id() const BOOST_NOEXCEPT { return id(); }
    native_handle_type native_handle() { }

    static unsigned hardware_concurrency() BOOST_NOEXCEPT { return 0u; }

private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(untracked_thread)
};

template class d2::trackable_thread<untracked_thread>;
} // end anonymous namespace
