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

struct thread {
    class id {
        boost::thread::id id_;

    public:
        id() : id_() { }

        D2_API /* implicit */ id(boost::thread::id const& thread_id);

        D2_API friend std::size_t unique_id(id const& self);
    };

    D2_API explicit thread(boost::function<void()> const& f);

    D2_API thread(BOOST_RV_REF(thread) other);

    D2_API friend void swap(thread& a, thread& b);

    D2_API void start();

    D2_API void join();

    D2_API friend std::size_t unique_id(thread const& self);

    D2_API id get_id() const;

private:
    D2_API bool is_initialized() const;

    boost::function<void()> f_;
    boost::scoped_ptr<boost::thread> actual_;
    boost::optional<id> id_;
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_THREAD_HPP
