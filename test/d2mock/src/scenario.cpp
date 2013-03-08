/**
 * This file implements the `check_scenario` function.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/scenario.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/set_algorithm.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>
#include <cstdlib>
#include <d2/api.hpp>
#include <d2/core/diagnostic.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/thread_id.hpp>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>


namespace bfs = boost::filesystem;

namespace d2mock {
namespace check_scenario_detail {
namespace {
//! Create a `d2::core::deadlocked_thread` from a `deadlocked_thread`.
d2::core::deadlocked_thread
to_d2_deadlocked_thread(deadlocked_thread const& thread) {
    return d2::core::deadlocked_thread(
                d2::ThreadId(thread.thread), thread.locks);
}

//! Create a `d2::core::potential_deadlock` from a `potential_deadlock`.
d2::core::potential_deadlock to_d2_deadlock(potential_deadlock const& dl) {
    d2::core::potential_deadlock ret;
    boost::transform(dl, std::inserter(ret, ret.end()),
                            to_d2_deadlocked_thread);
    return ret;
}

//! Print a `d2::core::potential_deadlock`.
void print_potential_deadlock(std::ostream& os,
                              d2::core::potential_deadlock const& dl) {
    BOOST_FOREACH(d2::core::deadlocked_thread const& thread, dl)
        os << thread << '\n';
    os << '\n';
}

template <typename Container>
bool check_scenario_results(Container const& expected, Container const& actual) {
    BOOST_ASSERT(boost::is_sorted(expected));
    BOOST_ASSERT(boost::is_sorted(actual));

    Container unexpected, unseen;
    boost::set_difference(expected, actual,
        std::inserter(unseen, unseen.end()));

    boost::set_difference(actual, expected,
        std::inserter(unexpected, unexpected.end()));

    if (!(unexpected.empty() && unseen.empty())) {
        if (expected.empty())
            std::cout << "expected no deadlocks\n\n";
        else {
            std::cout << "expected deadlocks:\n";
            BOOST_FOREACH(d2::core::potential_deadlock const& dl, expected)
                print_potential_deadlock(std::cout, dl);
        }

        if (actual.empty())
            std::cout << "no actual deadlocks\n\n";
        else {
            std::cout << "actual deadlocks:\n";
            BOOST_FOREACH(d2::core::potential_deadlock const& dl, actual)
                print_potential_deadlock(std::cout, dl);
        }

        BOOST_FOREACH(d2::core::potential_deadlock const& dl, unseen) {
            std::cout << "did not find expected deadlock:\n";
            print_potential_deadlock(std::cout, dl);
        }

        BOOST_FOREACH(d2::core::potential_deadlock const& dl, unexpected) {
            std::cout << "found unexpected deadlock:\n";
            print_potential_deadlock(std::cout, dl);
        }

        return false;
    }

    return true;
}
} // end anonymous namespace

/**
 * Run a scenario and verify the actual output with the expected output.
 *
 * @returns `EXIT_SUCCESS` or `EXIT_FAILURE`, depending on whether the
 *          actual deadlocks match the expected deadlocks or not.
 */
D2MOCK_DECL extern int
check_scenario(boost::function<void()> const& scenario,
               int argc, char const* argv[],
               std::vector<potential_deadlock> expected_) {

    bfs::path directory = argc > 1 ? bfs::path(argv[1]) :
                            bfs::temp_directory_path() / bfs::unique_path();

    if (bfs::exists(directory)) {
        std::cerr << "directory at " << directory << " already exists;"
                                                     " not overwriting it.\n";
        return EXIT_FAILURE;
    }

    if (d2::set_log_repository(directory.string())) {
        std::cerr << "unable to set the repository at " << directory << '\n';
        return EXIT_FAILURE;
    }

    d2::enable_event_logging();
    scenario();
    d2::disable_event_logging();
    d2::unset_log_repository();

    d2::core::synchronization_skeleton skeleton(directory);
    std::set<d2::core::potential_deadlock> actual, expected;
    boost::copy(skeleton.deadlocks(), std::inserter(actual, actual.end()));

    boost::transform(expected_,
        std::inserter(expected, expected.end()), to_d2_deadlock);

    return check_scenario_results(expected, actual) ? EXIT_SUCCESS
                                                    : EXIT_FAILURE;
}
} // end namespace check_scenario_detail
} // end namespace d2mock
