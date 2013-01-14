/**
 * This file defines mock mutex and thread implementations
 * for testing purposes.
 */

#ifndef TEST_MOCK_HPP
#define TEST_MOCK_HPP

#include <d2/detail/basic_atomic.hpp>

#include <boost/function.hpp>
#include <boost/move/move.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <cstdlib> // included for integration tests needing it
#include <string>


namespace mock {

extern bool begin_integration_test(int argc, char const* argv[],
                                   std::string const& source_file);
extern void end_integration_test();

class thread {
    boost::scoped_ptr<boost::thread> actual_;
    boost::function<void()> f_;

public:
    explicit thread(boost::function<void()> const& f);

    thread(BOOST_RV_REF(thread) other);

    friend void swap(thread& a, thread& b);

    void start();

    void join();
};

class mutex {
    static d2::detail::basic_atomic<std::size_t> counter;
    std::size_t id_;

public:
    mutex();

    void lock() const;

    void unlock() const;
};
} // end namespace mock

#endif // !TEST_MOCK_HPP
