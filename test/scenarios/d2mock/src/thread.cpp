/**
 * This file implements the `thread` class.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/this_thread.hpp>
#include <d2mock/thread.hpp>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/move/move.hpp>
#include <boost/swap.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <d2/api.hpp>


namespace d2mock {
namespace thread_detail {
static thread::id const NOT_A_THREAD = thread::id();

D2MOCK_DECL bool thread::was_started() const {
    return actual_ != NULL;
}

D2MOCK_DECL bool thread::has_id() const {
    return *id_ != NOT_A_THREAD;
}

D2MOCK_DECL thread::thread(boost::function<void()> const& f)
    : f_(f), actual_(), id_(new boost::atomic<thread::id>(NOT_A_THREAD))
{ }

D2MOCK_DECL thread::thread(BOOST_RV_REF(thread) other) {
    swap(*this, other);
}

D2MOCK_DECL thread::~thread() {
    if (was_started())
        join();
}

D2MOCK_DECL thread::id thread::get_id() const {
    BOOST_ASSERT(has_id());
    return *id_;
}

D2MOCK_DECL extern void swap(thread& a, thread& b) {
    boost::swap(a.f_, b.f_);
    boost::swap(a.actual_, b.actual_);
    boost::swap(a.id_, b.id_);
}

D2MOCK_DECL void thread::start() {
    BOOST_ASSERT_MSG(!was_started(),
        "starting a thread that was already started");

    boost::shared_ptr<boost::atomic<id> > child = this->id_;
    boost::function<void()> f = this->f_;
    auto thread_start = [f, child] {
        *child = this_thread::get_id(); // initialize the child thread's tid
        f();
    };

    bridge_.about_to_start();
    actual_.reset(new boost::thread(
                        d2::make_thread_function(bridge_, thread_start));
}

D2MOCK_DECL void thread::join() {
    BOOST_ASSERT(was_started());
    // Running actual_ initializes our thread id. Before we call
    // actual_->join(), we may or may not have a thread id, depending
    // on the thread scheduling.
    actual_->join();
    bridge_.just_joined();
}

D2MOCK_DECL extern std::size_t unique_id(thread const& self) {
    return unique_id(self.get_id());
}

D2MOCK_DECL extern std::size_t unique_id(thread_id const& self) {
    using boost::hash_value;
    return hash_value(self.id_);
}
} // end namespace thread_detail
} // end namespace d2mock
