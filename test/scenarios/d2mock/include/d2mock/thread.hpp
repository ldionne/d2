/*!
 * @file
 * This file defines the `d2mock::thread` class.
 */

#ifndef D2MOCK_THREAD_HPP
#define D2MOCK_THREAD_HPP

#include <d2mock/detail/decl.hpp>

#include <boost/function.hpp>
#include <boost/move/move.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <cstddef>


namespace d2mock {
struct thread {
    // We don't use d2::ThreadId directly to avoid leaking d2 as a dependency
    // to our clients, but there is a bijection between std::size_t and
    // d2::ThreadId anyway, so there is no loss of information.
    typedef std::size_t id;

    D2MOCK_DECL thread();
    D2MOCK_DECL thread(BOOST_RV_REF(thread) other);
    D2MOCK_DECL explicit thread(boost::function<void()> const& f);
    D2MOCK_DECL ~thread();
    D2MOCK_DECL thread& operator=(BOOST_RV_REF(thread) other);

    D2MOCK_DECL bool joinable();
    D2MOCK_DECL id get_id() const;

    D2MOCK_DECL void start();
    D2MOCK_DECL void join();
    D2MOCK_DECL void detach();
    D2MOCK_DECL void swap(thread& other);

    friend void swap(thread& a, thread& b) {
        a.swap(b);
    }

private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(thread)

    struct impl;
    boost::function<void()> f_;
    boost::scoped_ptr<impl> impl_;
};
} // end namespace d2mock

#endif // !D2MOCK_THREAD_HPP
