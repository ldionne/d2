/**
 * This file defines mock mutex and thread implementations
 * for testing purposes.
 */

#ifndef TEST_MOCK_HPP
#define TEST_MOCK_HPP

#include <d2/detail/basic_atomic.hpp>
#include <d2/sandbox/deadlock_diagnostic.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/move/move.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <string>
#include <vector>


namespace d2 {
namespace mock {

namespace detail {
struct AcquireStreak
    : boost::equality_comparable<
        AcquireStreak, sandbox::DeadlockDiagnostic::AcquireStreak
    >
{
    Thread thread_id;
    SyncObject lock1, lock2;

    // We make this a template to convert implicitly to Thread/SyncObject.
    template <typename Thread, typename Lock>
    AcquireStreak(Thread const& t, Lock const& l1, Lock const& l2)
        : thread_id(t), lock1(l1), lock2(l2)
    { }

    friend bool operator==(AcquireStreak const& self,
                sandbox::DeadlockDiagnostic::AcquireStreak const& other) {
        return self.lock1 == other.locks.front().lock_id &&
               self.lock2 == other.locks.back().lock_id &&
               self.thread_id == other.thread.thread_id;
    }
};

typedef std::vector<AcquireStreak> Deadlock;
typedef std::vector<Deadlock> Deadlocks;
} // end namespace detail

struct integration_test {
    integration_test(int argc, char const* argv[], std::string const& file);
    ~integration_test();

    void verify_deadlocks(detail::Deadlocks expected);

    void verify_deadlocks() {
        verify_deadlocks(detail::Deadlocks());
    }

private:
    boost::filesystem::path repo_;
};

struct thread {
    class id {
        boost::thread::id id_;

    public:
        id() { }
        /* implicit */ id(boost::thread::id const& thread_id);

        friend std::size_t unique_id(id const& self);
    };

    explicit thread(boost::function<void()> const& f);

    thread(BOOST_RV_REF(thread) other);

    friend void swap(thread& a, thread& b);

    void start();

    void join();

    friend std::size_t unique_id(thread const& self);

    thread::id get_id() const;

private:
    bool is_initialized() const;

    boost::function<void()> f_;
    boost::scoped_ptr<boost::thread> actual_;
    boost::optional<thread::id> id_;
};

namespace this_thread {
    extern thread::id get_id();
}

class mutex {
    static d2::detail::basic_atomic<std::size_t> counter;
    std::size_t id_;

public:
    mutex();

    void lock() const;

    void unlock() const;

    friend std::size_t unique_id(mutex const& self);
};

class recursive_mutex : public mutex {
public:
    void lock() const;

    void unlock() const;
};

} // end namespace mock
} // end namespace d2

#endif // !TEST_MOCK_HPP
