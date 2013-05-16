/*!
 * @file
 * This file defines the `d2::thread_function` class.
 */

#ifndef D2_THREAD_FUNCTION_HPP
#define D2_THREAD_FUNCTION_HPP

#include <d2/detail/thread_lifetime.hpp>

#include <boost/config.hpp>
#include <boost/fusion/functional/generation/make_fused.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/move/move.hpp>
#include <boost/move/utility.hpp>
#include <boost/type_traits/decay.hpp>
#include <boost/utility/result_of.hpp>


namespace d2 {
#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) &&                           \
    !defined(DYNO_DOXYGEN_INVOKED)

template <typename Function_>
class thread_function {
    typedef typename boost::decay<Function_>::type Function;

    // This handles pointers to functions correctly.
    template <typename F, typename Args>
    struct my_result_of
        : boost::result_of<
            typename boost::fusion::result_of::make_fused<Function>::type(Args)
        >
    { };

    Function function_;
    detail::thread_lifetime mutable lifetime_;

    BOOST_COPYABLE_AND_MOVABLE(thread_function)

public:
    explicit thread_function(detail::thread_lifetime const& lifetime)
        : lifetime_(lifetime)
    { }

    thread_function(detail::thread_lifetime const& lifetime, BOOST_RV_REF(Function) f)
        : function_(boost::move(f)), lifetime_(lifetime)
    { }

    thread_function(detail::thread_lifetime const& lifetime, Function const& f)
        : function_(boost::move(f)), lifetime_(lifetime)
    { }

    template <typename F>
    thread_function(detail::thread_lifetime const& lifetime, BOOST_FWD_REF(F) f)
        : function_(boost::forward<F>(f)), lifetime_(lifetime)
    { }

    thread_function(thread_function const& other)
        : function_(other.function_), lifetime_(other.lifetime_)
    { }

    thread_function(BOOST_RV_REF(thread_function) other)
        : function_(boost::move(other.function_)),
          lifetime_(boost::move(other.lifetime_))
    { }

    thread_function& operator=(BOOST_COPY_ASSIGN_REF(thread_function) other) {
        function_ = other.function_;
        lifetime_ = other.lifetime_;
        return *this;
    }

    thread_function& operator=(BOOST_RV_REF(thread_function) other) {
        function_ = boost::move(other.function_);
        lifetime_ = boost::move(other.lifetime_);
        return *this;
    }

    template <typename Sig> struct result;

    template <typename This>
    struct result<This()> : boost::result_of<Function()> { };
    template <typename This>
    struct result<This const()> : boost::result_of<Function const()> { };
    typename result<thread_function()>::type operator()() {
        lifetime_.just_started();
        return function_();
    }
    typename result<thread_function const()>::type operator()() const {
        lifetime_.just_started();
        return function_();
    }


    template <typename This, typename A0>
    struct result<This(A0)> : my_result_of<Function, boost::fusion::vector<A0> > { };
    template <typename This, typename A0>
    struct result<This const(A0)> : my_result_of<Function const, boost::fusion::vector<A0> > { };
    template <typename A0>
    typename result<thread_function(BOOST_FWD_REF(A0))>::type
    operator()(BOOST_FWD_REF(A0) a0) {
        lifetime_.just_started();
        return function_(boost::forward<A0>(a0));
    }
    template <typename A0>
    typename result<thread_function const(BOOST_FWD_REF(A0))>::type
    operator()(BOOST_FWD_REF(A0) a0) const {
        lifetime_.just_started();
        return function_(boost::forward<A0>(a0));
    }


    template <typename This, typename A0, typename A1>
    struct result<This(A0, A1)> : my_result_of<Function, boost::fusion::vector<A0, A1> > { };
    template <typename This, typename A0, typename A1>
    struct result<This const(A0, A1)> : my_result_of<Function const, boost::fusion::vector<A0, A1> > { };
    template <typename A0, typename A1>
    typename result<thread_function(BOOST_FWD_REF(A0), BOOST_FWD_REF(A1))>::type
    operator()(BOOST_FWD_REF(A0) a0, BOOST_FWD_REF(A1) a1) {
        lifetime_.just_started();
        return function_(boost::forward<A0>(a0), boost::forward<A1>(a1));
    }
    template <typename A0, typename A1>
    typename result<thread_function const(BOOST_FWD_REF(A0), BOOST_FWD_REF(A1))>::type
    operator()(BOOST_FWD_REF(A0) a0, BOOST_FWD_REF(A1) a1) const {
        lifetime_.just_started();
        return function_(boost::forward<A0>(a0), boost::forward<A1>(a1));
    }


    template <typename This, typename A0, typename A1, typename A2>
    struct result<This(A0, A1, A2)> : my_result_of<Function, boost::fusion::vector<A0, A1, A2> > { };
    template <typename This, typename A0, typename A1, typename A2>
    struct result<This const(A0, A1, A2)> : my_result_of<Function const, boost::fusion::vector<A0, A1, A2> > { };
    template <typename A0, typename A1, typename A2>
    typename result<thread_function(BOOST_FWD_REF(A0), BOOST_FWD_REF(A1), BOOST_FWD_REF(A2))>::type
    operator()(BOOST_FWD_REF(A0) a0, BOOST_FWD_REF(A1) a1, BOOST_FWD_REF(A2) a2) {
        lifetime_.just_started();
        return function_(boost::forward<A0>(a0), boost::forward<A1>(a1), boost::forward<A2>(a2));
    }
    template <typename A0, typename A1, typename A2>
    typename result<thread_function const(BOOST_FWD_REF(A0), BOOST_FWD_REF(A1), BOOST_FWD_REF(A2))>::type
    operator()(BOOST_FWD_REF(A0) a0, BOOST_FWD_REF(A1) a1, BOOST_FWD_REF(A2) a2) const {
        lifetime_.just_started();
        return function_(boost::forward<A0>(a0), boost::forward<A1>(a1), boost::forward<A2>(a2));
    }
};

#else // BOOST_NO_CXX11_VARIADIC_TEMPLATES

/*!
 * Wrapper over a function meant to be executed in a different thread.
 *
 * When the wrapper is created, it is given a `d2::detail::thread_lifetime` in
 * addition to the function that should be executed in a different thread.
 * When it is called, the wrapper calls the `just_started()` method on the
 * `d2::detail::thread_lifetime` instance and then forwards everything to the
 * wrapped function.
 *
 * If used within the `d2::detail::thread_lifetime` protocol, this will ensure
 * that the library can track thread lifetimes properly.
 *
 * @see `d2::detail::thread_lifetime`
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
    detail::thread_lifetime mutable lifetime_;

    typedef typename boost::decay<Function>::type Function_;
    Function_ f_;

public:
    explicit thread_function(detail::thread_lifetime const& lifetime)
        : lifetime_(lifetime)
    { }

    template <typename F>
    thread_function(detail::thread_lifetime const& lifetime, BOOST_FWD_REF(F) f)
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
make_thread_function(detail::thread_lifetime const& lifetime,
                     BOOST_FWD_REF(Function) f) {
    return thread_function<Function>(lifetime, boost::forward<Function>(f));
}
} // end namespace d2

#endif // !D2_THREAD_FUNCTION_HPP
