/**
 * This file implements the `d2mock/integration_test.hpp` header.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/integration_test.hpp>

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
#include <d2/api.hpp>
#include <d2/core/deadlock_diagnostic.hpp>
#include <d2/core/event_repository.hpp>
#include <d2/core/sync_skeleton.hpp>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>


namespace fs = boost::filesystem;

namespace d2mock {
namespace integration_test_detail {
namespace {
fs::path create_tmp_directory(fs::path const& test_source) {
    fs::path tmp = fs::temp_directory_path();
    fs::create_directory(tmp /= "d2_integration_tests");
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
D2MOCK_DECL integration_test::integration_test(int argc, char const* argv[],
                                               std::string const& test_source){
    repo_ = argc > 1 ? argv[1] : create_tmp_directory(test_source);
    if (d2::set_log_repository(repo_.string()))
        throw std::runtime_error(boost::str(boost::format(
            "setting the repository at %1% failed") % repo_));

    std::cout << boost::format("repository is at %1%\n") % repo_;
    d2::enable_event_logging();
}

D2MOCK_DECL integration_test::~integration_test() {
    d2::disable_event_logging();
}

namespace {
Streak make_streak(d2::core::AcquireStreak const& step) {
    Streak streak;
    streak.thread_id = step.thread;
    BOOST_FOREACH(d2::LockId const& lock, step.locks)
        streak.locks.push_back(lock);
    return streak;
}

Deadlock make_deadlock(d2::core::DeadlockDiagnostic const& diagnostic) {
    Deadlock deadlock;
    BOOST_FOREACH(d2::core::AcquireStreak const& streak, diagnostic.steps())
        deadlock.steps.push_back(make_streak(streak));
    return deadlock;
}

template <typename OutputIterator>
void verify_consume(std::vector<Deadlock> expected,
                    std::vector<Deadlock> actual,
                    OutputIterator missing) {
    if (expected.empty())
        return;

    Deadlock const& exp = expected.back();
    std::vector<Deadlock>::iterator
        it = boost::find_if(actual, [&](Deadlock const& act) {
            return exp.is_equivalent_to(act);
        });
    if (it == boost::end(actual))
        *missing++ = exp;
    else
        actual.erase(it);

    expected.pop_back();
    verify_consume(expected, actual, missing);
}
} // end anonymous namespace

D2MOCK_DECL void integration_test::verify_deadlocks(
                            std::initializer_list<Deadlock> const& expected) {
    d2::unset_log_repository();
    d2::core::EventRepository<> events(repo_);
    d2::core::SyncSkeleton<d2::core::EventRepository<> > skeleton(events);
    std::vector<Deadlock> actual;
    BOOST_FOREACH(d2::core::DeadlockDiagnostic const& diagnostic,
                                                        skeleton.deadlocks())
        actual.push_back(make_deadlock(diagnostic));

    std::vector<Deadlock> not_found, unexpected;
    verify_consume(expected, actual, std::back_inserter(not_found));
    verify_consume(actual, expected, std::back_inserter(unexpected));

    if (!unexpected.empty() || !not_found.empty()) {
        std::cout << "Expected deadlocks:\n";
        BOOST_FOREACH(Deadlock const& dl, expected)
            std::cout << dl << "\n";

        std::cout << "Actual deadlocks:\n";
        BOOST_FOREACH(Deadlock const& dl, actual)
            std::cout << dl << "\n";

        BOOST_FOREACH(Deadlock const& dl, not_found)
            std::cout << "did not find expected deadlock:\n" << dl << '\n';

        BOOST_FOREACH(Deadlock const& dl, unexpected)
            std::cout << "found unexpected deadlock:\n" << dl << '\n';

        throw std::logic_error("failed integration test");
    }
}

D2MOCK_DECL std::ostream& operator<<(std::ostream& os, Streak const& self) {
    os << "thread " << self.thread_id << " acquires ";
    boost::copy(self.locks, std::ostream_iterator<d2::LockId>(os, ", "));
    return os;
}

/**
 * Return whether two deadlocks are equivalent.
 *
 * @todo Instead of copying, rotate back the other deadlock.
 */
D2MOCK_DECL bool Deadlock::is_equivalent_to(Deadlock other) const {
    typedef std::vector<Streak>::size_type size_type;
    for (size_type n = 0; n < this->steps.size(); ++n) {
        if (this->steps == other.steps)
            return true;
        std::rotate(other.steps.begin(), other.steps.begin() + 1,
                    other.steps.end());
    }
    return false;
}

D2MOCK_DECL std::ostream& operator<<(std::ostream& os, Deadlock const& self) {
    boost::copy(self.steps, std::ostream_iterator<Streak>(os, "\n"));
    return os;
}
} // end namespace mock
} // end namespace d2
