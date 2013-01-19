/**
 * Implementation of a mock mutex and a mock thread class
 * for testing purposes.
 */

#include "mock.hpp"
#include <d2/detail/basic_atomic.hpp>
#include <d2/event_repository.hpp>
#include <d2/logging.hpp>
#include <d2/sandbox/sync_skeleton.hpp>

#include <algorithm>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/move/move.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>


namespace fs = boost::filesystem;

namespace d2 {
namespace mock {

namespace detail {
    inline bool operator==(Deadlock const& a,
                           sandbox::DeadlockDiagnostic const& b) {
        return a.size() == b.steps().size() &&
               std::equal(a.begin(), a.end(), b.steps().begin());
    }

    inline bool operator==(sandbox::DeadlockDiagnostic const& a,
                           Deadlock const& b) {
        return b == a;
    }
} // end namespace detail

namespace {
fs::path create_tmp_directory(fs::path const& test_source) {
    fs::path tmp = fs::temp_directory_path();
    fs::create_directory(tmp /= "d2_integration_tests_for_deadlock_analysis");
    tmp /= fs::path(test_source).stem();

    unsigned int i = 0;
    while (fs::exists(tmp))
        tmp.replace_extension(boost::lexical_cast<std::string>(i++));
    fs::create_directory(tmp);

    return tmp;
}
} // end anonymous namespace

/**
 * Setup a small environment for the integration test to take place.
 *
 * @note If the setup fails, an exception will be thrown and the test will
 *       fail (unless the test catches the exception, which it should not).
 */
integration_test::integration_test(int argc, char const* argv[],
                                   std::string const& test_source) {
    repo_ = argc > 1 ? argv[1] : create_tmp_directory(test_source);
    if (d2::set_log_repository(repo_.string()))
        throw std::runtime_error((boost::format(
            "setting the repository at %1% failed\n") % repo_).str());

    std::cout << boost::format("repository is at %1%\n") % repo_;
    d2::enable_event_logging();
}

integration_test::~integration_test() {
    d2::disable_event_logging();
}

namespace detail {
    /**
     * Functor returning whether an element is contained in a sequence.
     */
    template <typename Container>
    struct contained_in_type {
        Container const& container_;

        explicit contained_in_type(Container const& c) : container_(c) { }

        typedef bool result_type;

        template <typename T>
        result_type operator()(T const& element) const {
            return boost::find(container_, element) != boost::end(container_);
        }
     };

     template <typename Container>
     contained_in_type<Container> contained_in(Container const& c) {
        return contained_in_type<Container>(c);
     }

    /**
     * Return whether the arbitrary sequence `s1` is a non-strict subset of
     * the arbitrary sequence `s2`.
     */
    template <typename S1, typename S2>
    bool is_non_strict_subset_of(S1 const& s1, S2 const& s2) {
        return boost::algorithm::all_of(s1, contained_in(s2));
    }

    /**
     * Return whether two arbitrary sequences contain the same elements.
     */
    template <typename S1, typename S2>
    bool contain_same_elements(S1 const& s1, S2 const& s2) {
        return is_non_strict_subset_of(s1, s2) &&
               is_non_strict_subset_of(s2, s1);
    }
} // end namespace detail

void integration_test::verify_deadlocks(detail::Deadlocks expected) {
    BOOST_ASSERT(fs::exists(repo_));
    EventRepository<> events(repo_);
    sandbox::SyncSkeleton<EventRepository<> > skeleton(events);
    auto actual = skeleton.deadlocks();
    BOOST_ASSERT_MSG(detail::contain_same_elements(actual, expected),
        "the found deadlocks were not as expected");
}


namespace detail {
class thread_functor_wrapper {
    thread::id parent_;
    boost::function<void()> f_;

public:
    explicit thread_functor_wrapper(boost::function<void()> const& f)
        : parent_(this_thread::get_id()), f_(f)
    { }

    void operator()() const {
        thread::id child = this_thread::get_id();
        d2::notify_start(parent_, child);
        f_();
        d2::notify_join(parent_, child);
    }
};
} // end namespace detail

bool thread::is_initialized() const {
    return id_ && actual_;
}

thread::thread(boost::function<void()> const& f)
    : f_(detail::thread_functor_wrapper(f))
{ }

thread::thread(BOOST_RV_REF(thread) other) : f_(boost::move(other.f_)) {
    using std::swap;
    swap(actual_, other.actual_);
    swap(id_, other.id_);
}

thread::id thread::get_id() const {
    BOOST_ASSERT(is_initialized());
    return thread::id(actual_->get_id());
}

void swap(thread& a, thread& b) {
    using std::swap;
    swap(a.f_, b.f_);
    swap(a.actual_, b.actual_);
    swap(a.id_, b.id_);
}

void thread::start() {
    BOOST_ASSERT_MSG(!is_initialized(),
        "starting a thread that is already started");
    actual_.reset(new boost::thread(f_));
    id_ = actual_->get_id();
}

void thread::join() {
    BOOST_ASSERT_MSG(is_initialized(), "joining a thread that is not started");
    actual_->join();
}

extern std::size_t unique_id(thread const& self) {
    BOOST_ASSERT(self.is_initialized());
    return unique_id(*self.id_);
}


thread::id::id(boost::thread::id const& thread_id)
    : id_(thread_id)
{ }

extern std::size_t unique_id(thread::id const& self) {
    using boost::hash_value;
    return hash_value(self.id_);
}

namespace this_thread {
    extern thread::id get_id() {
        return thread::id(boost::this_thread::get_id());
    }
}


mutex::mutex()
    : id_(counter++)
{ }

void mutex::lock() const {
    d2::notify_acquire(this_thread::get_id(), *this);
}

void mutex::unlock() const {
    d2::notify_release(this_thread::get_id(), *this);
}

extern std::size_t unique_id(mutex const& self) {
    return self.id_;
}

d2::detail::basic_atomic<std::size_t> mutex::counter(0);


void recursive_mutex::lock() const {
    d2::notify_recursive_acquire(this_thread::get_id(), *this);
}

void recursive_mutex::unlock() const {
    d2::notify_recursive_release(this_thread::get_id(), *this);
}

} // end namespace mock
} // end namespace d2
