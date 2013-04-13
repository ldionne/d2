/*!
 * @file
 * This file defines the `d2::trackable_thread` class.
 */

#ifndef D2_TRACKABLE_THREAD_HPP
#define D2_TRACKABLE_THREAD_HPP

#include <d2/core/thread_id.hpp>
#include <d2/thread_function.hpp>
#include <d2/thread_lifetime.hpp>

#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/utility/swap.hpp>


namespace d2 {
namespace trackable_thread_detail {
    struct lifetime_as_member {
        lifetime_as_member() BOOST_NOEXCEPT { }

        lifetime_as_member(BOOST_RV_REF(lifetime_as_member) other)
            : lifetime_(boost::move(other.lifetime_))
        { }

        lifetime_as_member& operator=(BOOST_RV_REF(lifetime_as_member) other) {
            lifetime_ = boost::move(other.lifetime_);
            return *this;
        }

        friend void swap(lifetime_as_member& self, lifetime_as_member & other){
            boost::swap(self.lifetime_, other.lifetime_);
        }

        thread_lifetime lifetime_;

    private:
        BOOST_MOVABLE_BUT_NOT_COPYABLE(lifetime_as_member)
    };
} // end namespace trackable_thread_detail

/*!
 * Wrapper over a standard conformant thread class to add tracking of the
 * thread's lifetime.
 *
 * @warning If the wrapped thread is not standard conformant, the behavior is
 *          undefined.
 *
 * @internal Private inheritance is for the base-from-member idiom.
 */
template <typename Thread>
class trackable_thread
    : private trackable_thread_detail::lifetime_as_member,
      public Thread
{
    typedef trackable_thread_detail::lifetime_as_member lifetime_as_member;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(trackable_thread)

    template <typename Function>
    thread_function<Function> forward_to_thread(BOOST_FWD_REF(Function) f) {
        this->lifetime_.about_to_start();
        return make_thread_function(this->lifetime_,
                                    boost::forward<Function>(f));
    }

public:
    trackable_thread() BOOST_NOEXCEPT { }

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <typename F, typename ...Args>
    explicit trackable_thread(BOOST_FWD_REF(F) f,
                              BOOST_FWD_REF(Args) ...args)
        : Thread(forward_to_thread(boost::forward<F>(f)),
                                   boost::forward<Args>(args)...)
    { }
#else
    template <typename F>
    explicit trackable_thread(BOOST_FWD_REF(F) f)
        : Thread(forward_to_thread(boost::forward<F>(f)))
    { }

    template <typename F, typename A0>
    trackable_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0)
        : Thread(forward_to_thread(boost::forward<F>(f)),
                                   boost::forward<A0>(a0))
    { }

    template <typename F, typename A0, typename A1>
    trackable_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0,
                                         BOOST_FWD_REF(A1) a1)
        : Thread(forward_to_thread(boost::forward<F>(f)),
                                   boost::forward<A0>(a0),
                                   boost::forward<A1>(a1))
    { }

    template <typename F, typename A0, typename A1, typename A2>
    trackable_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0,
                                         BOOST_FWD_REF(A1) a1,
                                         BOOST_FWD_REF(A2) a2)
        : Thread(forward_to_thread(boost::forward<F>(f)),
                                   boost::forward<A0>(a0),
                                   boost::forward<A1>(a1),
                                   boost::forward<A2>(a2))
    { }

    template <typename F, typename A0, typename A1, typename A2, typename A3>
    trackable_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0,
                                         BOOST_FWD_REF(A1) a1,
                                         BOOST_FWD_REF(A2) a2,
                                         BOOST_FWD_REF(A3) a3)
        : Thread(forward_to_thread(boost::forward<F>(f)),
                                   boost::forward<A0>(a0),
                                   boost::forward<A1>(a1),
                                   boost::forward<A2>(a2),
                                   boost::forward<A3>(a3))
    { }
#endif // !BOOST_NO_CXX11_VARIADIC_TEMPLATES

    trackable_thread(BOOST_RV_REF(trackable_thread) other) BOOST_NOEXCEPT
        : lifetime_as_member(
            boost::move(static_cast<lifetime_as_member&>(other))),
          Thread(boost::move(static_cast<Thread&>(other)))
    { }

    trackable_thread&
    operator=(BOOST_RV_REF(trackable_thread) other) BOOST_NOEXCEPT {
        lifetime_as_member::operator=(
            boost::move(static_cast<lifetime_as_member&>(other)));
        Thread::operator=(boost::move(static_cast<Thread&>(other)));
        return *this;
    }

    void swap(trackable_thread& other) BOOST_NOEXCEPT {
        boost::swap(static_cast<lifetime_as_member&>(*this),
                    static_cast<lifetime_as_member&>(other));
        boost::swap(static_cast<Thread&>(*this), static_cast<Thread&>(other));
    }

    friend void swap(trackable_thread& x, trackable_thread& y) BOOST_NOEXCEPT{
        x.swap(y);
    }

    void join() {
        Thread::join();
        this->lifetime_.just_joined();
    }

    void detach() {
        Thread::detach();
        this->lifetime_.just_detached();
    }
};

/*!
 * Mixin augmenting its derived class with `d2` tractability.
 *
 * By implementing `join_impl()` and `detach_impl()` methods, the derived
 * class automatically gets augmented with `join()` and `detach()` methods
 * notifying `d2` after forwarding to the `*_impl()` implementation.
 *
 * When a new thread is created, the function executed in the new thread must
 * also be wrapped by calling `trackable_thread_mixin::get_thread_function()`.
 *
 * Intended usage of this class goes as follow:
 * @code
 *
 *  class my_thread : public d2::trackable_thread_mixin<my_thread> {
 *      friend class d2::trackable_thread_mixin<my_thread>;
 *
 *      void join_impl() {
 *          // ...
 *      }
 *
 *      void detach_impl() {
 *          // ...
 *      }
 *
 *  public:
 *      template <typename Function, typename ...Args>
 *      explicit my_thread(Function&& f, Args&& ...args) {
 *          typedef d2::thread_function<Function> Function_;
 *          Function_ f_ = this->get_thread_function(
 *                                              boost::forward<Function>(f));
 *          // start the thread with Function_ and f_ normally
 *      }
 *
 *      my_thread(my_thread&& other)
 *          : trackable_thread_mixin_(
 *              boost::move(static_cast<trackable_thread_mixin_&>(other)))
 *      { }
 *
 *      my_thread& operator=(my_thread&& other) {
 *          trackable_thread_mixin_::operator=(boost::move(other));
 *          // ...
 *      }
 *  };
 *
 * @endcode
 */
template <typename Derived>
class trackable_thread_mixin {
    thread_lifetime lifetime_;

protected:
    template <typename Function>
    thread_function<Function> get_thread_function(BOOST_FWD_REF(Function) f) {
        lifetime_.about_to_start();
        return make_thread_function(lifetime_, boost::forward<Function>(f));
    }

    typedef trackable_thread_mixin trackable_thread_mixin_;

public:
    void join() {
        static_cast<Derived*>(this)->join_impl();
        lifetime_.just_joined();
    }

    void detach() {
        static_cast<Derived*>(this)->detach_impl();
        lifetime_.just_detached();
    }
};
} // end namespace d2

#endif // !D2_TRACKABLE_THREAD_HPP
