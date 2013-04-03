/*!
 * @file
 * This file contains unit tests for the `d2::trackable_thread` class.
 */

#include <d2/trackable_thread.hpp>

#include <boost/move/move.hpp>
#include <gtest/gtest.h>


namespace {
/*!
 * @internal
 * Dummy class with a standard thread interface for unit testing purpose.
 */
struct thread {
    class id { };
    typedef void native_handle_type;

    thread() BOOST_NOEXCEPT { }
    template <typename F> explicit thread(BOOST_FWD_REF(F) f, ...) { }
    ~thread() { }

    thread(BOOST_RV_REF(thread) t) BOOST_NOEXCEPT { }

    thread& operator=(BOOST_RV_REF(thread) t) BOOST_NOEXCEPT { return *this; }

    void swap(thread& t) BOOST_NOEXCEPT { }

    bool joinable() const BOOST_NOEXCEPT { return false; }
    void join() { }
    void detach() { }
    id get_id() const BOOST_NOEXCEPT { return id(); }
    native_handle_type native_handle() { }

    static unsigned hardware_concurrency() BOOST_NOEXCEPT { return 0u; }

private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(thread)
};

TEST(trackable_thread, pending) {
    FAIL();
}
} // end anonymous namespace
