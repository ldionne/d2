/*!
 * @file
 * This file contains unit tests for the `d2::thread_function` class.
 */

#include <d2/thread_function.hpp>
#include <d2/thread_lifetime.hpp>


namespace {
template <typename F>
struct instantiate_thread_function {
    typedef d2::thread_function<F> ThreadFunction;
    d2::thread_lifetime lifetime;

    instantiate_thread_function() {
        int i;

        ThreadFunction f(lifetime);
        f();
        f(i);
        f(i, i);
        f(i, i, i);
    }
};

struct void_functor_any_args_non_const {
    typedef void result_type;
    result_type operator()(...) { }
};
struct void_functor_any_args_const {
    typedef void result_type;
    result_type operator()(...) const { }
};

template struct instantiate_thread_function<void_functor_any_args_non_const>;
template struct instantiate_thread_function<void_functor_any_args_const>;
template struct instantiate_thread_function<void(...)>;
template struct instantiate_thread_function<void(&)(...)>;
template struct instantiate_thread_function<void(*)(...)>;
} // end anonymous namespace
