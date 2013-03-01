/**
 * This file defines the `thread` class.
 */

#ifndef D2MOCK_THREAD_HPP
#define D2MOCK_THREAD_HPP

#include <d2mock/detail/decl.hpp>

#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/move/move.hpp>
#include <boost/operators.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace d2mock {
namespace thread_detail {
struct thread_id : boost::equality_comparable<thread_id> {
    thread_id() : id_() { }

    /*implicit*/ thread_id(boost::thread::id const& tid) : id_(tid) { }

    D2MOCK_DECL friend std::size_t unique_id(thread_id const& self);

    friend bool operator==(thread_id const& self, thread_id const& other) {
        return self.id_ == other.id_;
    }

private:
    boost::thread::id id_;
};

struct thread {
    typedef thread_id id;

    D2MOCK_DECL explicit thread(boost::function<void()> const& f);

    D2MOCK_DECL thread(BOOST_RV_REF(thread) other);

    D2MOCK_DECL ~thread();

    D2MOCK_DECL friend void swap(thread& a, thread& b);

    D2MOCK_DECL void start();

    D2MOCK_DECL void join();

    D2MOCK_DECL friend std::size_t unique_id(thread const& self);

    D2MOCK_DECL id get_id() const;

private:
    D2MOCK_DECL bool has_id() const;
    D2MOCK_DECL bool was_started() const;

    boost::function<void()> f_;
    boost::scoped_ptr<boost::thread> actual_;
    boost::shared_ptr<boost::atomic<id> > id_;
};
} // end namespace thread_detail

using thread_detail::thread;
} // end namespace d2mock

#endif // !D2MOCK_THREAD_HPP
