/*!
 * @file
 * This file implements the `d2::trackable_thread` and the
 * `d2::thread_function` classes.
 */

#ifndef D2_TRACKABLE_THREAD_HPP
#define D2_TRACKABLE_THREAD_HPP

#include <d2/api.hpp>
#include <d2/detail/inherit_constructors.hpp>
#include <d2/trackable_sync_object.hpp>

#include <boost/fusion/functional/adapter/unfused.hpp>
#include <boost/fusion/functional/generation/make_fused.hpp>
#include <boost/move/utility.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/utility/result_of.hpp>
#include <cstddef>


namespace d2 {
namespace trackable_thread_detail {
namespace fsn = boost::fusion;

template <typename Function_>
class thread_function_impl {
    typedef typename boost::decay<Function_>::type Function;
    typedef std::size_t ThreadId;

    static ThreadId this_thread_id() {
        return trackable_sync_object_detail::this_thread_id();
    }

    Function function_;
    ThreadId parent_;

public:
    thread_function_impl()
        : parent_(this_thread_id())
    { }

    /*!
     * @internal
     * Implicit is required because we perform a long-shot conversion from
     * `Function` to `thread_function_impl` when we create a
     * `d2::thread_function`.
     */
    /* implicit */ thread_function_impl(Function const& f)
        : function_(f)
        , parent_(this_thread_id())
    { }

    /* implicit */ thread_function_impl(BOOST_RV_REF(Function) f)
        : function_(boost::move(f))
        , parent_(this_thread_id())
    { }

    template <typename Sig>
    struct result;

    template <typename This, typename Args>
    struct result<This(Args)>
        : boost::result_of<
            typename fsn::result_of::make_fused<Function&>::type(Args)
        >
    { };

    template <typename This, typename Args>
    struct result<This const(Args)>
        : boost::result_of<
            typename fsn::result_of::make_fused<Function const&>::type(Args)
        >
    { };

    template <typename Args>
    typename boost::result_of<thread_function_impl(BOOST_FWD_REF(Args))>::type
    operator()(BOOST_FWD_REF(Args) args) {
        ThreadId const child = this_thread_id();
        d2::notify_start(parent_, child);
        return fsn::make_fused(function_)(boost::forward<Args>(args));
    }

    template <typename Args>
    typename boost::result_of<
        thread_function_impl const(BOOST_FWD_REF(Args))
    >::type operator()(BOOST_FWD_REF(Args) args) const {
        ThreadId const child = this_thread_id();
        d2::notify_start(parent_, child);
        return fsn::make_fused(function_)(boost::forward<Args>(args));
    }
};
} // end namespace trackable_thread_detail

/*!
 * Wrapper over a function meant to be executed in a different thread.
 *
 * When it is called, the wrapper will notify the library that a thread was
 * started. All functions executed in a different thread should be wrapped
 * with this to ensure the library can track the threads correctly.
 *
 * Intended usage is as follows:
 * @code
 *
 *  class thread {
 *  public:
 *      template <typename F, typename ...Args>
 *      void start(F&& f_, Args&& ...args) {
 *          d2::thread_function<F> f(boost::forward<F>(f_));
 *          // now, proceed with `f` exactly as if you would normally
 *      }
 *  };
 *
 * @endcode
 *
 * @tparam Function
 *         The type of the wrapped function. Before being used, `Function` is
 *         decayed. Thus, `Function` being a function type, a reference to a
 *         function or something similar is not an error because it will
 *         be taken away by `boost::decay`.
 *
 * @internal This should be a template alias, but we want to stay portable.
 */
template <typename Function>
class thread_function
    : public boost::fusion::unfused<
        trackable_thread_detail::thread_function_impl<Function>,
        true /* allow nullary */
    >
{
    typedef boost::fusion::unfused<
                trackable_thread_detail::thread_function_impl<Function>,
                true /* allow nullary */
            > Base;
public:
    thread_function() { }
    D2_INHERIT_CONSTRUCTORS(thread_function, Base)
};

//! Helper to create a `d2::thread_function` with a deduced `Function` type.
template <typename Function>
thread_function<Function> make_thread_function(BOOST_FWD_REF(Function) f) {
    return thread_function<Function>(boost::forward<Function>(f));
}

/*!
 *
 */
template <typename Thread>
class trackable_thread {
public:

    void join() {
        Thread::join();
        d2::notify_join(this_thread_id(), joined.get_id());
    }

    void detach() {
        //! @todo Add support for detached threads. issue #18
        Thread::detach();
    }
};
} // end namespace d2

#endif // !D2_TRACKABLE_THREAD_HPP
