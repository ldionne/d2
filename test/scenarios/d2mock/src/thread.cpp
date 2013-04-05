/*!
 * @file
 * This file implements the `d2mock::thread` class.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/thread.hpp>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/move/utility.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility/swap.hpp>
#include <cstddef>
#include <d2/trackable_thread.hpp>
#include <dyno/uniquely_identifiable.hpp>


namespace d2mock {
typedef d2::trackable_thread<boost::thread> thread_impl_base;
struct member_d2_tid {
    boost::shared_ptr<boost::atomic<std::size_t> > d2_tid;
};

struct thread::impl : member_d2_tid, thread_impl_base {
    impl(boost::function<void()> const& f)
        : thread_impl_base([=] {
            BOOST_ASSERT(!d2_tid);

            using dyno::unique_id;
            std::size_t const tid = unique_id(dyno::this_thread::get_id());
            d2_tid.reset(new boost::atomic<std::size_t>(tid));

            f();
        })
    { }
};

D2MOCK_DECL thread::thread()
    : f_(), impl_(0)
{ }

D2MOCK_DECL thread::thread(BOOST_RV_REF(thread) other)
    : f_(), impl_(0)
{
    boost::swap(f_, other.f_);
    boost::swap(impl_, other.impl_);
}

D2MOCK_DECL thread::thread(boost::function<void()> const& f)
    : f_(f), impl_(0)
{ }

D2MOCK_DECL thread::~thread() { }

D2MOCK_DECL thread& thread::operator=(BOOST_RV_REF(thread) other) {
    boost::swap(f_, other.f_);
    boost::swap(impl_, other.impl_);
    return *this;
}


D2MOCK_DECL bool thread::joinable() {
    BOOST_ASSERT_MSG(impl_, "thread::joinable(): was not started");
    return impl_->joinable();
}

D2MOCK_DECL thread::id thread::get_id() const {
    BOOST_ASSERT_MSG(impl_, "thread::get_id(): was not started");
    return impl_->get_id();
}


D2MOCK_DECL void thread::start() {
    BOOST_ASSERT_MSG(!impl_, "thread::start(): was already started");
    impl_.reset(new impl(f_));
}

D2MOCK_DECL void thread::join() {
    BOOST_ASSERT_MSG(impl_, "thread::join(): was not started");
    impl_->join();
}

D2MOCK_DECL void thread::detach() {
    BOOST_ASSERT_MSG(impl_, "thread::detach(): was not started");
    impl_->detach();
}

D2MOCK_DECL void thread::swap(thread& other) {
    boost::swap(f_, other.f_);
    boost::swap(impl_, other.impl_);
}

D2MOCK_DECL std::size_t thread::d2_unique_id() const {
    BOOST_ASSERT_MSG(impl_ && impl_->d2_tid,
        "thread::d2_unique_id(): was not started");
    return *impl_->d2_tid;
}
} // end namespace d2mock
