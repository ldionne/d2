/**
 * This file defines a mock thread class.
 */

#ifndef D2_MOCK_THREAD_HPP
#define D2_MOCK_THREAD_HPP

#include <d2/detail/config.hpp>

#include <boost/function.hpp>
#include <boost/move/move.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace d2 {
namespace mock {

struct D2_API thread {
    class D2_API id {
        boost::thread::id id_;

    public:
        id() : id_() { }

        /* implicit */ id(boost::thread::id const& thread_id);

        D2_API friend std::size_t unique_id(id const& self);
    };

    explicit thread(boost::function<void()> const& f);

    thread(BOOST_RV_REF(thread) other);

    D2_API friend void swap(thread& a, thread& b);

    void start();

    void join();

    D2_API friend std::size_t unique_id(thread const& self);

    id get_id() const;

private:
    bool is_initialized() const;

    boost::function<void()> f_;
    boost::scoped_ptr<boost::thread> actual_;
    boost::optional<id> id_;
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_THREAD_HPP
