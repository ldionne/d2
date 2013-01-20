/**
 * This file implements the `mock/thread.hpp` header.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/logging.hpp>
#include <d2/mock/this_thread.hpp>
#include <d2/mock/thread.hpp>

#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/move/move.hpp>
#include <boost/swap.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace d2 {
namespace mock {

namespace detail {
class thread_functor_wrapper {
    thread::id parent_;
    boost::function<void()> f_;

public:
    explicit thread_functor_wrapper(boost::function<void()> const& f)
        : parent_(this_thread::get_id()), f_(f)
    { }

    void operator()() const {
        thread::id child = this_thread::get_id();
        notify_start(parent_, child);
        f_();
        notify_join(parent_, child);
    }
};
} // end namespace detail

bool thread::is_initialized() const {
    return id_ && actual_;
}

thread::thread(boost::function<void()> const& f)
    : f_(detail::thread_functor_wrapper(f))
{ }

thread::thread(BOOST_RV_REF(thread) other) : f_(boost::move(other.f_)) {
    boost::swap(actual_, other.actual_);
    boost::swap(id_, other.id_);
}

thread::id thread::get_id() const {
    BOOST_ASSERT(is_initialized());
    return thread::id(actual_->get_id());
}

D2_API extern void swap(thread& a, thread& b) {
    boost::swap(a.f_, b.f_);
    boost::swap(a.actual_, b.actual_);
    boost::swap(a.id_, b.id_);
}

void thread::start() {
    BOOST_ASSERT_MSG(!is_initialized(),
        "starting a thread that is already started");
    actual_.reset(new boost::thread(f_));
    id_ = actual_->get_id();
}

void thread::join() {
    BOOST_ASSERT_MSG(is_initialized(), "joining a thread that is not started");
    actual_->join();
}

D2_API extern std::size_t unique_id(thread const& self) {
    BOOST_ASSERT(self.is_initialized());
    return unique_id(*self.id_);
}

thread::id::id(boost::thread::id const& thread_id)
    : id_(thread_id)
{ }

D2_API extern std::size_t unique_id(thread::id const& self) {
    using boost::hash_value;
    return hash_value(self.id_);
}

} // end namespace mock
} // end namespace d2
