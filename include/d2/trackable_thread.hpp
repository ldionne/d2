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
} // end namespace d2

#endif // !D2_TRACKABLE_THREAD_HPP
