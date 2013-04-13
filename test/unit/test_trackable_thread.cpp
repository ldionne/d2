/*!
 * @file
 * This file contains unit tests for the `d2::trackable_thread` class.
 */

#include <d2/trackable_thread.hpp>

#include <boost/move/move.hpp>
#include <boost/utility/swap.hpp>


namespace {
struct thread_base {
    class id { };
    typedef void native_handle_type;

    bool joinable() const BOOST_NOEXCEPT { return false; }

    id get_id() const BOOST_NOEXCEPT { return id(); }
    native_handle_type native_handle() { }

    static unsigned hardware_concurrency() BOOST_NOEXCEPT { return 0u; }
};

struct wrapped_thread : thread_base {
    wrapped_thread() BOOST_NOEXCEPT { }
    template <typename F> explicit wrapped_thread(BOOST_FWD_REF(F), ...) { }
    ~wrapped_thread() { }

    wrapped_thread(BOOST_RV_REF(wrapped_thread)) BOOST_NOEXCEPT { }

    wrapped_thread&
    operator=(BOOST_RV_REF(wrapped_thread)) BOOST_NOEXCEPT { return *this; }

    void swap(wrapped_thread&) BOOST_NOEXCEPT { }
    friend void swap(wrapped_thread&, wrapped_thread&) BOOST_NOEXCEPT { }

    void join() { }
    void detach() { }

private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(wrapped_thread)
};
} // end anonymous namespace

template class d2::trackable_thread<wrapped_thread>;

namespace {
typedef d2::trackable_thread<wrapped_thread> wrapped_trackable_thread;

struct thread_mixin : thread_base, d2::trackable_thread_mixin<thread_mixin> {
    thread_mixin() BOOST_NOEXCEPT { }
    template <typename F> explicit thread_mixin(BOOST_FWD_REF(F) f, ...) {
        this->get_thread_function(boost::forward<F>(f));
    }
    ~thread_mixin() { }

    thread_mixin(BOOST_RV_REF(thread_mixin) other) BOOST_NOEXCEPT
        : trackable_thread_mixin_(
                    boost::move(static_cast<trackable_thread_mixin_&>(other)))
    { }

    thread_mixin& operator=(BOOST_RV_REF(thread_mixin) other) BOOST_NOEXCEPT {
        trackable_thread_mixin_::operator=(
                   boost::move(static_cast<trackable_thread_mixin_&>(other)));
        return *this;
    }

    void swap(thread_mixin&) BOOST_NOEXCEPT { }
    friend void swap(thread_mixin&, thread_mixin&) BOOST_NOEXCEPT { }

private:
    friend class d2::trackable_thread_mixin<thread_mixin>;
    void join_impl() { }
    void detach_impl() { }

    BOOST_MOVABLE_BUT_NOT_COPYABLE(thread_mixin)
};

template <typename Thread>
struct use_thread {
    struct void_functor {
        void operator()() const { }
    };

    use_thread() {
        Thread default_t;
        Thread move_t(boost::move(default_t));
        Thread t((void_functor()));
        t.join();
        t.detach();
        boost::swap(t, t);
        t = boost::move(t);
    }
};

template struct use_thread<wrapped_trackable_thread>;
template struct use_thread<thread_mixin>;
} // end anonymous namespace
