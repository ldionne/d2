/**
 * Implementation of a mock mutex and a mock thread class
 * for testing purposes.
 */

#include "mock.hpp"
#include <d2/detail/basic_atomic.hpp>
#include <d2/logging.hpp>

#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace boost {
    inline std::size_t unique_id(boost::thread::id thread_id) {
        return hash_value(thread_id);
    }
}

namespace mock {

namespace detail {
class thread_functor_wrapper {
    boost::thread::id parent_;
    boost::function<void()> f_;

public:
    explicit thread_functor_wrapper(boost::function<void()> const& f)
        : parent_(boost::this_thread::get_id()), f_(f)
    { }

    void operator()() const {
        boost::thread::id child = boost::this_thread::get_id();
        d2::notify_start(parent_, child);
        f_();
        d2::notify_join(parent_, child);
    }
};
} // end namespace detail

thread::thread(boost::function<void()> const& f)
    : f_(detail::thread_functor_wrapper(f))
{ }

thread::thread(BOOST_RV_REF(thread) other) : f_(boost::move(other.f_)) {
    actual_.reset(other.actual_.get());
    other.actual_.reset();
}

void swap(thread& a, thread& b) {
    using std::swap;
    swap(a.f_, b.f_);
    swap(a.actual_, b.actual_);
}

void thread::start() {
    BOOST_ASSERT_MSG(!actual_, "starting an already started thread");
    actual_.reset(new boost::thread(f_));
}

void thread::join() {
    BOOST_ASSERT_MSG(actual_, "joining a thread that is not started");
    actual_->join();
    actual_.reset();
}


mutex::mutex() : id_(counter++) { }

void mutex::lock() const {
   d2::notify_acquire(id_, boost::this_thread::get_id());
}

void mutex::unlock() const {
    d2::notify_release(id_, boost::this_thread::get_id());
}

d2::detail::basic_atomic<std::size_t> mutex::counter(0);

} // end namespace mock
