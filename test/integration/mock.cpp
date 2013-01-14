/**
 * Implementation of a mock mutex and a mock thread class
 * for testing purposes.
 */

#include "mock.hpp"
#include <d2/detail/basic_atomic.hpp>
#include <d2/logging.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/move/move.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <iostream>
#include <string>


namespace mock {

namespace {
std::string create_tmp_directory(std::string const& path_to_test_source) {
    namespace fs = boost::filesystem;
    fs::path tmp = fs::temp_directory_path();
    fs::create_directory(tmp /= "d2_integration_tests_for_deadlock_analysis");
    tmp /= fs::path(path_to_test_source).stem();

    unsigned int i = 0;
    while (fs::exists(tmp))
        tmp.replace_extension(boost::lexical_cast<std::string>(i++));
    fs::create_directory(tmp);

    return tmp.string();
}
} // end anonymous namespace

extern void begin_integration_test(int argc, char const* argv[],
                                          std::string const& test_source) {
    std::string log_repo(argc > 1 ? argv[1] :
                                    create_tmp_directory(test_source));
    std::cout << "logging repository: \"" << log_repo << '"' << std::endl;
    d2::set_log_repository(log_repo);
    d2::enable_event_logging();
}

extern void end_integration_test() {
    d2::disable_event_logging();
}

namespace detail {
class thread_functor_wrapper {
    boost::thread::id parent_;
    boost::function<void()> f_;

public:
    explicit thread_functor_wrapper(boost::function<void()> const& f)
        : parent_(boost::this_thread::get_id()), f_(f)
    { }

    void operator()() const {
        boost::thread::id child = boost::this_thread::get_id();
        d2::notify_start(hash_value(parent_), hash_value(child));
        f_();
        d2::notify_join(hash_value(parent_), hash_value(child));
    }
};
} // end namespace detail

thread::thread(boost::function<void()> const& f)
    : f_(detail::thread_functor_wrapper(f))
{ }

thread::thread(BOOST_RV_REF(thread) other) : f_(boost::move(other.f_)) {
    swap(actual_, other.actual_);
}

void swap(thread& a, thread& b) {
    using std::swap;
    swap(a.f_, b.f_);
    swap(a.actual_, b.actual_);
}

void thread::start() {
    BOOST_ASSERT_MSG(!actual_, "starting an already started thread");
    actual_.reset(new boost::thread(f_));
}

void thread::join() {
    BOOST_ASSERT_MSG(actual_, "joining a thread that is not started");
    actual_->join();
    actual_.reset();
}


mutex::mutex() : id_(counter++) { }

void mutex::lock() const {
   d2::notify_acquire(hash_value(boost::this_thread::get_id()), id_);
}

void mutex::unlock() const {
    d2::notify_release(hash_value(boost::this_thread::get_id()), id_);
}

d2::detail::basic_atomic<std::size_t> mutex::counter(0);

} // end namespace mock
