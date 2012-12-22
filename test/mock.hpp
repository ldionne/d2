/**
 * This file defines mock mutex and thread implementations
 * for testing purposes.
 */

#ifndef TEST_MOCK_HPP
#define TEST_MOCK_HPP

#include <d2/detail/basic_atomic.hpp>
#include <d2/logging.hpp>

#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/move/move.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace boost {
    inline std::size_t unique_id(boost::thread::id thread_id) {
        return hash_value(thread_id);
    }
}

namespace {

template <typename F>
class thread_functor_wrapper {
    boost::thread::id parent_;
    F f_;

public:
    explicit thread_functor_wrapper(F const& f)
        : parent_(boost::this_thread::get_id()), f_(f)
    { }

    void operator()() const {
        boost::thread::id child = boost::this_thread::get_id();
        d2::notify_start(parent_, child);
        f_();
        d2::notify_join(parent_, child);
    }
};

template <typename F>
thread_functor_wrapper<F> make_thread_functor_wrapper(F const& f) {
    return thread_functor_wrapper<F>(f);
}

class mock_thread {
    boost::scoped_ptr<boost::thread> actual_;
    boost::function<void()> f_;

public:
    template <typename F>
    explicit mock_thread(F const& f)
        : f_(make_thread_functor_wrapper(f))
    { }

    inline mock_thread(BOOST_RV_REF(mock_thread) other)
        : f_(boost::move(other.f_)) {
        actual_.reset(other.actual_.get());
        other.actual_.reset();
    }

    friend void swap(mock_thread& a, mock_thread& b) {
        using std::swap;
        swap(a.f_, b.f_);
        swap(a.actual_, b.actual_);
    }

    inline void start() {
        BOOST_ASSERT_MSG(!actual_, "starting an already started thread");
        actual_.reset(new boost::thread(f_));
    }

    inline void join() {
        BOOST_ASSERT_MSG(actual_, "joining a thread that is not started");
        actual_->join();
        actual_.reset();
    }
};

class mock_mutex {
    static d2::detail::basic_atomic<std::size_t> counter;
    std::size_t id_;

public:
    inline mock_mutex()
        : id_(counter++)
    { }

    inline void lock() const {
        d2::notify_acquire(id_, boost::this_thread::get_id());
    }

    inline void unlock() const {
        d2::notify_release(id_, boost::this_thread::get_id());
    }
};

d2::detail::basic_atomic<std::size_t> mock_mutex::counter(0);

} // end anonymous namespace

#endif // !TEST_MOCK_HPP
