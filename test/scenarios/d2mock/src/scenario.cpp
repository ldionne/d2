/*!
 * @file
 * This file implements the `d2mock::check_scenario` function.
 */

#define D2MOCK_SOURCE
#include <d2mock/detail/decl.hpp>
#include <d2mock/scenario.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <cstdlib>
#include <d2/api.hpp>
#include <d2/core/diagnostic.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/core/thread_id.hpp>
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
                // We must NOT construct the ThreadId before now, because
                // we must be sure the thread has been started (e.g. after
                // the test) for it to have a thread identifier.
                d2::ThreadId(thread.thread),
                thread.locks.begin(), thread.locks.end() - 1,
                thread.locks.back());
}

//! Create a `d2::core::potential_deadlock` from a `potential_deadlock`.
d2::core::potential_deadlock to_d2_deadlock(potential_deadlock const& dl) {
    std::vector<d2::core::deadlocked_thread> threads;
    boost::transform(dl, std::back_inserter(threads), to_d2_deadlocked_thread);
    return d2::core::potential_deadlock(boost::move(threads));
}

//! Output the deadlocks that are in `expected` but not in `actual` to `result`.
template <typename Deadlocks, typename OutputIterator>
void consume_equivalent_deadlocks(Deadlocks expected, Deadlocks actual,
                                  OutputIterator result) {
    while (!expected.empty()) {
        typename Deadlocks::const_reference exp = expected.back();
        typename Deadlocks::iterator it = boost::find_if(actual,
            [&](typename Deadlocks::const_reference act) {
                return exp.is_equivalent_to(act);
            });
        if (it == actual.end())
            *result++ = exp;
        else
            actual.erase(it);
        expected.pop_back();
    }
}

bool check_scenario_results(
                    std::vector<d2::core::potential_deadlock> const& expected,
                    std::vector<d2::core::potential_deadlock> const& actual) {
    std::vector<d2::core::potential_deadlock> unexpected, unseen;
    consume_equivalent_deadlocks(expected, actual, std::back_inserter(unseen));
    consume_equivalent_deadlocks(actual, expected,
                                            std::back_inserter(unexpected));

    if (!(unexpected.empty() && unseen.empty())) {
        if (expected.empty())
            std::cout << "expected no deadlocks\n\n";
        else {
            std::cout << "expected deadlocks:\n";
            BOOST_FOREACH(d2::core::potential_deadlock const& dl, expected)
                std::cout << dl << '\n';
        }

        if (actual.empty())
            std::cout << "no actual deadlocks\n\n";
        else {
            std::cout << "actual deadlocks:\n";
            BOOST_FOREACH(d2::core::potential_deadlock const& dl, actual)
                std::cout << dl << '\n';
        }

        BOOST_FOREACH(d2::core::potential_deadlock const& dl, unseen)
            std::cout << "did not find expected deadlock:\n" << dl << '\n';

        BOOST_FOREACH(d2::core::potential_deadlock const& dl, unexpected)
            std::cout << "found unexpected deadlock:\n" << dl << '\n';

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
    std::vector<d2::core::potential_deadlock> actual, expected;
    boost::copy(skeleton.deadlocks(), std::back_inserter(actual));
    boost::transform(expected_, std::back_inserter(expected), to_d2_deadlock);

    if (!check_scenario_results(expected, actual)) {
        std::cout << "Segmentation graph\n"
                     "------------------\n";
        skeleton.print_segmentation_graph(std::cout);

        std::cout << "\n\nLock graph\n"
                         "----------\n";
        skeleton.print_lock_graph(std::cout);
        std::cout << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
} // end namespace check_scenario_detail
} // end namespace d2mock
