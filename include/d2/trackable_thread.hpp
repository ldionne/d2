/*!
 * @file
 * This file defines the `d2::trackable_thread` class.
 */

#ifndef D2_TRACKABLE_THREAD_HPP
#define D2_TRACKABLE_THREAD_HPP

#include <d2/detail/thread_lifetime.hpp>
#include <d2/thread_function.hpp>

#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/utility/swap.hpp>


namespace d2 {
/*!
 * Class providing basic facilities to track a thread with `d2`.
 */
class trackable_thread {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(trackable_thread)
    detail::thread_lifetime lifetime_;

public:
    trackable_thread() BOOST_NOEXCEPT { }

    trackable_thread(BOOST_RV_REF(trackable_thread) other)
        : lifetime_(boost::move(other.lifetime_))
    { }

    trackable_thread& operator=(BOOST_RV_REF(trackable_thread) other) {
        lifetime_ = boost::move(other.lifetime_);
        return *this;
    }

    friend void swap(trackable_thread& self, trackable_thread & other){
        boost::swap(self.lifetime_, other.lifetime_);
    }

    template <typename Function>
    thread_function<Function> get_thread_function(BOOST_FWD_REF(Function) f) {
        lifetime_.about_to_start();
        return make_thread_function(lifetime_, boost::forward<Function>(f));
    }

    void notify_join() {
        lifetime_.just_joined();
    }

    void notify_detach() {
        lifetime_.just_detached();
    }
};
} // end namespace d2

#endif // !D2_TRACKABLE_THREAD_HPP
