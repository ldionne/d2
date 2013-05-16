/*!
 * @file
 * This file defines the `d2::standard_thread` class.
 */

#ifndef D2_STANDARD_THREAD_HPP
#define D2_STANDARD_THREAD_HPP

#include <d2/access.hpp>
#include <d2/core/thread_id.hpp>
#include <d2/trackable_thread.hpp>

#include <boost/config.hpp>
#include <boost/move/move.hpp>
#include <boost/utility/swap.hpp>


namespace d2 {
/*!
 * Wrapper over a standard conformant thread class to add tracking of the
 * thread's lifetime.
 *
 * @warning If the wrapped thread is not standard conformant, the behavior is
 *          undefined.
 */
template <typename Thread>
class standard_thread : private trackable_thread, public Thread {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(standard_thread)
    using trackable_thread::get_thread_function;

public:
    standard_thread() BOOST_NOEXCEPT { }

#ifndef BOOST_NO_CXX11_VARIADIC_TEMPLATES
    template <typename F, typename ...Args>
    explicit standard_thread(BOOST_FWD_REF(F) f,
                             BOOST_FWD_REF(Args) ...args)
        : Thread(get_thread_function(boost::forward<F>(f)),
                                     boost::forward<Args>(args)...)
    { }
#else
    template <typename F>
    explicit standard_thread(BOOST_FWD_REF(F) f)
        : Thread(get_thread_function(boost::forward<F>(f)))
    { }

    template <typename F, typename A0>
    standard_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0)
        : Thread(get_thread_function(boost::forward<F>(f)),
                                     boost::forward<A0>(a0))
    { }

    template <typename F, typename A0, typename A1>
    standard_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0,
                                        BOOST_FWD_REF(A1) a1)
        : Thread(get_thread_function(boost::forward<F>(f)),
                                     boost::forward<A0>(a0),
                                     boost::forward<A1>(a1))
    { }

    template <typename F, typename A0, typename A1, typename A2>
    standard_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0,
                                        BOOST_FWD_REF(A1) a1,
                                        BOOST_FWD_REF(A2) a2)
        : Thread(get_thread_function(boost::forward<F>(f)),
                                     boost::forward<A0>(a0),
                                     boost::forward<A1>(a1),
                                     boost::forward<A2>(a2))
    { }

    template <typename F, typename A0, typename A1, typename A2, typename A3>
    standard_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(A0) a0,
                                        BOOST_FWD_REF(A1) a1,
                                        BOOST_FWD_REF(A2) a2,
                                        BOOST_FWD_REF(A3) a3)
        : Thread(get_thread_function(boost::forward<F>(f)),
                                     boost::forward<A0>(a0),
                                     boost::forward<A1>(a1),
                                     boost::forward<A2>(a2),
                                     boost::forward<A3>(a3))
    { }
#endif // !BOOST_NO_CXX11_VARIADIC_TEMPLATES

    standard_thread(BOOST_RV_REF(standard_thread) other) BOOST_NOEXCEPT
        : trackable_thread(
            boost::move(static_cast<trackable_thread&>(other))),
          Thread(boost::move(static_cast<Thread&>(other)))
    { }

    standard_thread&
    operator=(BOOST_RV_REF(standard_thread) other) BOOST_NOEXCEPT {
        trackable_thread::operator=(
            boost::move(static_cast<trackable_thread&>(other)));
        Thread::operator=(boost::move(static_cast<Thread&>(other)));
        return *this;
    }

    void swap(standard_thread& other) BOOST_NOEXCEPT {
        boost::swap(static_cast<trackable_thread&>(*this),
                    static_cast<trackable_thread&>(other));
        boost::swap(static_cast<Thread&>(*this), static_cast<Thread&>(other));
    }

    friend void swap(standard_thread& x, standard_thread& y) BOOST_NOEXCEPT {
        x.swap(y);
    }

    void join() {
        Thread::join();
        this->notify_join();
    }

    void detach() {
        Thread::detach();
        this->notify_detach();
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
 * also be wrapped by calling `standard_thread_mixin::get_thread_function()`.
 *
 * Intended usage of this class goes as follow:
 * @code
 *
 *  class my_thread : public d2::standard_thread_mixin<my_thread> {
 *      friend class d2::access;
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
 *          : standard_thread_mixin_(
 *              boost::move(static_cast<standard_thread_mixin_&>(other)))
 *      { }
 *
 *      my_thread& operator=(my_thread&& other) {
 *          standard_thread_mixin_::operator=(boost::move(other));
 *          // ...
 *      }
 *  };
 *
 * @endcode
 */
template <typename Derived>
class standard_thread_mixin : private trackable_thread {
    BOOST_MOVABLE_BUT_NOT_COPYABLE(standard_thread_mixin)

protected:
    using trackable_thread::get_thread_function;
    typedef standard_thread_mixin standard_thread_mixin_;

public:
    standard_thread_mixin() BOOST_NOEXCEPT { }

    standard_thread_mixin(BOOST_RV_REF(standard_thread_mixin) other)
                                                                BOOST_NOEXCEPT
        : trackable_thread(
            boost::move(static_cast<trackable_thread&>(other)))
    { }

    standard_thread_mixin&
    operator=(BOOST_RV_REF(standard_thread_mixin) other) BOOST_NOEXCEPT {
        trackable_thread::operator=(
            boost::move(static_cast<trackable_thread&>(other)));
        return *this;
    }

    void swap(standard_thread_mixin& other) BOOST_NOEXCEPT {
        boost::swap(static_cast<trackable_thread&>(*this),
                    static_cast<trackable_thread&>(other));
    }

    friend void
    swap(standard_thread_mixin& x, standard_thread_mixin& y) BOOST_NOEXCEPT {
        x.swap(y);
    }

    void join() {
        access::join_impl(static_cast<Derived&>(*this));
        this->notify_join();
    }

    void detach() {
        access::detach_impl(static_cast<Derived&>(*this));
        this->notify_detach();
    }
};
} // end namespace d2

#endif // !D2_STANDARD_THREAD_HPP
