/**
 * This file defines mock mutex and thread implementations
 * for testing purposes.
 */

#ifndef TEST_MOCK_HPP
#define TEST_MOCK_HPP

#include <d2/detail/basic_atomic.hpp>
#include <d2/logging.hpp>

#include <cstddef>


namespace {

class mock_thread {
    static d2::detail::basic_atomic<std::size_t> counter;
    std::size_t id_;

public:
    inline mock_thread() : id_(counter++) { }

    friend std::size_t unique_id(mock_thread const& self) {
        return self.id_;
    }

    inline void start(mock_thread const& child) const {
        d2::notify_start(*this, child);
    }

    inline void join(mock_thread const& child) const {
        d2::notify_join(*this, child);
    }
};

d2::detail::basic_atomic<std::size_t> mock_thread::counter(0);

class mock_mutex {
    static d2::detail::basic_atomic<std::size_t> counter;
    std::size_t id_;

public:
    inline mock_mutex() : id_(counter++) { }

    inline void lock_in(mock_thread const& thread) const {
        d2::notify_acquire(*this, thread);
    }

    inline void unlock_in(mock_thread const& thread) const {
        d2::notify_release(*this, thread);
    }

    friend std::size_t unique_id(mock_mutex const& self) {
        return self.id_;
    }
};

d2::detail::basic_atomic<std::size_t> mock_mutex::counter(0);

} // end anonymous namespace

#endif // !TEST_MOCK_HPP
