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

public:
    trackable_thread() BOOST_NOEXCEPT { }

    template <typename F, typename ...Args>
    explicit trackable_thread(BOOST_FWD_REF(F) f, BOOST_FWD_REF(Args) ...args)
        : lifetime_as_member(),
          Thread(
            (this->lifetime_.about_to_start(),
                make_thread_function(this->lifetime_, boost::forward<F>(f))),
                 boost::forward<Args>(args)...)
    { }

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

#ifdef D2MOCK_TRACKABLE_SYNC_OBJECT_ACCESS
protected:
    /*!
     * @internal
     * This is a hack because we need to access the `d2` thread id for unit
     * testing purposes.
     *
     * @todo Implement this.
     */
    ThreadId get_d2_id() const {
        return ThreadId();
    }
#endif
};
} // end namespace d2

#endif // !D2_TRACKABLE_THREAD_HPP
