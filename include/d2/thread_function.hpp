/*!
 * @file
 * This file defines the `d2::thread_function` class.
 */

#ifndef D2_THREAD_FUNCTION_HPP
#define D2_THREAD_FUNCTION_HPP

#include <d2/thread_lifetime.hpp>

#include <boost/config.hpp>
#include <boost/fusion/functional/adapter/unfused.hpp>
#include <boost/fusion/functional/generation/make_fused.hpp>
#include <boost/move/utility.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/utility/result_of.hpp>


namespace d2 {
#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) &&                           \
    !defined(DYNO_DOXYGEN_INVOKED)

namespace thread_function_detail {
namespace fsn = boost::fusion;

template <typename Function_>
class thread_function_impl {
    typedef typename boost::decay<Function_>::type Function;

    Function function_;
    thread_lifetime mutable lifetime_;

public:
    explicit thread_function_impl(thread_lifetime const& lifetime)
        : lifetime_(lifetime)
    { }

    thread_function_impl(thread_lifetime const& lifetime, Function const& f)
        : function_(f), lifetime_(lifetime)
    { }

    thread_function_impl(thread_lifetime const& lifetime,
                         BOOST_RV_REF(Function) f)
        : function_(boost::move(f)), lifetime_(lifetime)
    { }

    template <typename Sig>
    struct result;

    template <typename This>
    struct result<This()>
        : boost::result_of<
            typename fsn::result_of::make_fused<Function&>::type()
        >
    { };

    template <typename This>
    struct result<This const()>
        : boost::result_of<
            typename fsn::result_of::make_fused<Function const&>::type()
        >
    { };

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
    typename result<thread_function_impl(BOOST_FWD_REF(Args))>::type
    operator()(BOOST_FWD_REF(Args) args) {
        lifetime_.just_started();
        return fsn::make_fused(function_)(boost::forward<Args>(args));
    }

    template <typename Args>
    typename result<thread_function_impl const(BOOST_FWD_REF(Args))>::type
    operator()(BOOST_FWD_REF(Args) args) const {
        lifetime_.just_started();
        return fsn::make_fused(function_)(boost::forward<Args>(args));
    }

    typename result<thread_function_impl()>::type operator()() {
        lifetime_.just_started();
        return fsn::make_fused(function_)();
    }

    typename result<thread_function_impl const()>::type operator()() const {
        lifetime_.just_started();
        return fsn::make_fused(function_)();
    }
};
} // end namespace thread_function_detail

template <typename Function>
class thread_function
    : public boost::fusion::unfused<
        thread_function_detail::thread_function_impl<Function>,
        true /* allow nullary */
    >
{
    typedef thread_function_detail::thread_function_impl<Function> Impl;
    typedef boost::fusion::unfused<Impl, true /* allow nullary */> Base;

public:
    explicit thread_function(thread_lifetime const& lifetime)
        : Base(Impl(lifetime))
    { }

    template <typename F>
    thread_function(thread_lifetime const& lifetime, BOOST_FWD_REF(F) f)
        : Base(Impl(lifetime, boost::forward<F>(f)))
    { }
};

#else // BOOST_NO_CXX11_VARIADIC_TEMPLATES

/*!
 * Wrapper over a function meant to be executed in a different thread.
 *
 * When the wrapper is created, it is given a `d2::thread_lifetime` in
 * addition to the function that should be executed in a different thread.
 * When it is called, the wrapper calls the `just_started()` method on the
 * `d2::thread_lifetime` instance and then forwards everything to the wrapped
 * function.
 *
 * If used within the `d2::thread_lifetime` protocol, this will ensure that
 * the library can track thread lifetimes properly.
 *
 * @see `d2::thread_lifetime`
 *
 * @tparam Function
 *         The type of the wrapped function. Before being used, `Function` is
 *         decayed. Thus, `Function` being a function type, a reference to a
 *         function or something similar is not an error because it will
 *         be taken away by `boost::decay`.
 *
 * @internal This should be a template alias, but we want to support C++03.
 */
template <typename Function>
class thread_function {
    thread_lifetime mutable lifetime_;

    typedef typename boost::decay<Function>::type Function_;
    Function_ f_;

public:
    explicit thread_function(thread_lifetime const& lifetime)
        : lifetime_(lifetime)
    { }

    template <typename F>
    thread_function(thread_lifetime const& lifetime, BOOST_FWD_REF(F) f)
        : lifetime_(lifetime), f_(boost::forward<F>(f))
    { }

    template <typename Sig>
    struct result;

    template <typename This, typename ...Args>
    struct result<This(Args...)>
        : boost::result_of<Function_(Args...)>
    { };

    template <typename ...Args>
    typename result<thread_function(BOOST_FWD_REF(Args)...)>::type
    operator()(BOOST_FWD_REF(Args) ...args) {
        lifetime_.just_started();
        return f_(boost::forward<Args>(args)...);
    }


    template <typename This, typename ...Args>
    struct result<This const(Args...)>
        : boost::result_of<Function_ const(Args...)>
    { };

    template <typename ...Args>
    typename result<thread_function const(BOOST_FWD_REF(Args)...)>::type
    operator()(BOOST_FWD_REF(Args) ...args) const {
        lifetime_.just_started();
        return f_(boost::forward<Args>(args)...);
    }
};
#endif // BOOST_NO_CXX11_VARIADIC_TEMPLATES

//! Helper to create a `d2::thread_function` with a deduced `Function` type.
template <typename Function>
thread_function<Function>
make_thread_function(thread_lifetime const& lifetime,
                     BOOST_FWD_REF(Function) f) {
    return thread_function<Function>(lifetime, boost::forward<Function>(f));
}
} // end namespace d2

#endif // !D2_THREAD_FUNCTION_HPP
