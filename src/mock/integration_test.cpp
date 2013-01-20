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
#include <boost/foreach.hpp>
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
        throw std::runtime_error(boost::str(boost::format(
            "setting the repository at %1% failed\n") % repo_));

    std::cout << boost::format("repository is at %1%\n") % repo_;
    enable_event_logging();
}

integration_test::~integration_test() {
    disable_event_logging();
}

namespace detail {
integration_test::Streak
make_streak(sandbox::DeadlockDiagnostic::AcquireStreak const& step) {
    integration_test::Streak streak;
    streak.thread_id = step.thread.thread_id;
    typedef sandbox::DeadlockDiagnostic::LockInformation LockInfo;
    BOOST_FOREACH(LockInfo const& lock_info, step.locks)
        streak.locks.push_back(lock_info.lock_id);
    return streak;
}

integration_test::Deadlock
make_deadlock(sandbox::DeadlockDiagnostic const& diagnostic) {
    integration_test::Deadlock deadlock;
    BOOST_FOREACH(sandbox::DeadlockDiagnostic::AcquireStreak const& streak,
                                                        diagnostic.steps())
        deadlock.steps.push_back(make_streak(streak));
    return deadlock;
}
} // end namespace detail

void integration_test::verify_deadlocks(
                            std::initializer_list<Deadlock> const& expected) {
    unset_log_repository();
    EventRepository<> events(repo_);
    sandbox::SyncSkeleton<EventRepository<> > skeleton(events);
    std::vector<Deadlock> actual;
    BOOST_FOREACH(sandbox::DeadlockDiagnostic const& diagnostic,
                                                        skeleton.deadlocks())
        actual.push_back(detail::make_deadlock(diagnostic));

    if (expected.size() != actual.size())
        throw std::logic_error(boost::str(boost::format(
            "expected %1% deadlocks, but got %2%\n")
            % expected.size() % actual.size()));

    BOOST_FOREACH(Deadlock const& exp, expected)
        if (boost::find(actual, exp) == boost::end(actual))
            throw std::logic_error("expected deadlock that was not found\n");

    BOOST_FOREACH(Deadlock const& act, actual)
        if (boost::find(expected, act) == boost::end(expected))
            throw std::logic_error("found a deadlock that was not expected\n");
}

} // end namespace mock
} // end namespace d2
