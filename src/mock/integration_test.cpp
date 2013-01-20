/**
 * This file implements the `mock/integration_test.hpp` header.
 */

#define D2_SOURCE
#include <d2/api.hpp>
#include <d2/detail/config.hpp>
#include <d2/detail/getter.hpp>
#include <d2/event_repository.hpp>
#include <d2/mock/integration_test.hpp>
#include <d2/sandbox/deadlock_diagnostic.hpp>
#include <d2/sandbox/sync_skeleton.hpp>

#include <algorithm>
#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>


namespace fs = boost::filesystem;

namespace d2 {
namespace mock {

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
    if (set_log_repository(repo_.string()))
        throw std::runtime_error((boost::format(
            "setting the repository at %1% failed\n") % repo_).str());

    std::cout << boost::format("repository is at %1%\n") % repo_;
    enable_event_logging();
}

integration_test::~integration_test() {
    disable_event_logging();
}


D2_API extern bool
operator==(integration_test::Streak const& self,
           sandbox::DeadlockDiagnostic::AcquireStreak const& other) {
    typedef sandbox::DeadlockDiagnostic::LockInformation LockInfo;
    return self.thread_id == other.thread.thread_id &&
           self.locks == (other.locks |
           boost::adaptors::transformed(d2::detail::get(&LockInfo::lock_id)));
}

D2_API extern bool
operator==(integration_test::Deadlock const& self,
           sandbox::DeadlockDiagnostic const& other) {
    return self.steps.size() == other.steps().size() &&
           std::equal(boost::begin(self.steps), boost::end(self.steps),
                      boost::begin(other.steps()));
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

void integration_test::verify_deadlocks(
                            std::initializer_list<Deadlock> const& expected) {
    BOOST_ASSERT(fs::exists(repo_));
    EventRepository<> events(repo_);
    sandbox::SyncSkeleton<EventRepository<> > skeleton(events);
    auto actual = skeleton.deadlocks();
    BOOST_ASSERT_MSG(detail::contain_same_elements(actual, expected),
        "the found deadlocks were not as expected");
}

} // end namespace mock
} // end namespace d2
