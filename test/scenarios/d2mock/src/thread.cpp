/*!
 * @file
 * This file implements the `d2mock::thread` class.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/thread.hpp>

#include <boost/assert.hpp>
#include <boost/function.hpp>
#include <boost/move/utility.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility/swap.hpp>
#include <d2/trackable_thread.hpp>
#include <dyno/uniquely_identifiable.hpp>


namespace d2mock {
typedef d2::trackable_thread<boost::thread> thread_impl_base;

struct thread::impl : thread_impl_base {
    impl() { }

    explicit impl(boost::function<void()> const& f)
        : thread_impl_base(f)
    { }

    thread::id get_id() {
        using d2::unique_id;
        return unique_id(thread_impl_base::get_d2_id());
    }
};

D2MOCK_DECL thread()
    : f_(), impl_(0)
{ }

D2MOCK_DECL thread(BOOST_RV_REF(thread) other)
    : f_(), impl_(0)
{
    boost::swap(f_, other.f_);
    boost::swap(impl_, other.impl_);
}

D2MOCK_DECL explicit thread(boost::function<void()> const& f)
    : f_(f), impl_(0)
{ }

D2MOCK_DECL ~thread() { }

D2MOCK_DECL thread& operator=(BOOST_RV_REF(thread) other) {
    boost::swap(f_, other.f_);
    boost::swap(impl_, other.impl_);
}


D2MOCK_DECL bool joinable() {
    BOOST_ASSERT_MSG(impl_, "thread::joinable(): was not started");
    return impl_->joinable();
}

D2MOCK_DECL id get_id() const {
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
} // end namespace thread_detail
} // end namespace d2mock
