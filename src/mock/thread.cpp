/**
 * This file implements the `mock/thread.hpp` header.
 */

#define D2_SOURCE
#include <d2/api.hpp>
#include <d2/detail/config.hpp>
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

static thread::id const NOT_A_THREAD = thread::id();

D2_API bool thread::was_started() const {
    return actual_ != NULL;
}

D2_API bool thread::has_id() const {
    return *id_ != NOT_A_THREAD;
}

D2_API thread::thread(boost::function<void()> const& f)
    : f_(f), actual_(), id_(new detail::basic_atomic<thread::id>(NOT_A_THREAD))
{ }

D2_API thread::thread(BOOST_RV_REF(thread) other) {
    swap(*this, other);
}

D2_API thread::~thread() {
    join();
}

D2_API thread::id thread::get_id() const {
    BOOST_ASSERT(has_id());
    return *id_;
}

D2_API extern void swap(thread& a, thread& b) {
    boost::swap(a.f_, b.f_);
    boost::swap(a.actual_, b.actual_);
    boost::swap(a.id_, b.id_);
}

D2_API void thread::start() {
    BOOST_ASSERT_MSG(!was_started(),
        "starting a thread that was already started");

    thread::id parent = this_thread::get_id();
    boost::shared_ptr<detail::basic_atomic<id> > child = this->id_;
    boost::function<void()> f = this->f_;
    actual_.reset(new boost::thread([f, parent, child] {
        *child = this_thread::get_id(); // initialize the child thread's id
                            // atomic cast to thread::id
        notify_start(parent, static_cast<thread::id>(*child));
        f();
    }));
}

D2_API void thread::join() {
    BOOST_ASSERT(was_started());
    // Running actual_ initializes our thread id. Before we call
    // actual_->join(), we may or may not have a thread id, depending
    // on the thread scheduling.
    actual_->join();
    notify_join(this_thread::get_id(), get_id());
}

D2_API extern std::size_t unique_id(thread const& self) {
    return unique_id(self.get_id());
}


D2_API thread::id::id(boost::thread::id const& thread_id)
    : id_(thread_id)
{ }

D2_API extern std::size_t unique_id(thread::id const& self) {
    using boost::hash_value;
    return hash_value(self.id_);
}

D2_API extern bool operator==(thread::id const& self, thread::id const& other){
    return self.id_ == other.id_;
}

} // end namespace mock
} // end namespace d2
