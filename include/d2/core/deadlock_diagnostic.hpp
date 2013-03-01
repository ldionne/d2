/**
 * This file defines classes to present deadlock potential diagnostics.
 */

#ifndef D2_CORE_DEADLOCK_DIAGNOSTIC_HPP
#define D2_CORE_DEADLOCK_DIAGNOSTIC_HPP

#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <boost/optional.hpp>
#include <cstddef>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>


namespace d2 {
namespace deadlock_diagnostic_detail {
/**
 * Class representing a sequence of lock acquisitions by a single thread.
 */
struct AcquireStreak {
    template <typename Iterator>
    AcquireStreak(ThreadId thread, Iterator first_lock, Iterator last_lock)
        : thread(thread), locks(first_lock, last_lock)
    { }

    friend std::ostream& operator<<(std::ostream&, AcquireStreak const&);

    ThreadId thread;
    // sorted in order of acquisition
    std::vector<LockId> locks;
};

/**
 * Class representing the diagnostic of a deadlock potential.
 *
 * @internal This class is not very optimized (sets created by some getters,
 *           etc...), but the truth is: we don't care! This is nowhere near
 *           a performance critical part and the number of deadlock potentials
 *           is likely to be small.
 */
struct DeadlockDiagnostic {
    template <typename Iterator>
    DeadlockDiagnostic(Iterator first_streak, Iterator last_streak)
        : steps_(first_streak, last_streak)
    { }

    typedef std::vector<AcquireStreak> Steps;

    /**
     * Range of `AcquireStreak`s showing the state required by each
     * involved thread in order for the system as a whole to become
     * deadlocked.
     *
     * The steps leading each thread to its deadlock state are guaranteed to
     * be ordered relatively to each other in a way that it creates a chain.
     * Example of correctly ordered:
     *  Steps of thread 1: A, B
     *  Steps of thread 2: B, C
     *  Steps of thread 3: C, A
     * This creates the `chain' [A, (B], [C), A]. Any other configuration
     * yielding a valid chain is considered valid. Anything else is not.
     *
     * @note The relative order thing is because it makes it easier for an
     *       user to spot a deadlock when the steps leading to it are ordered
     *       that way.
     */
    Steps& steps() {
        return steps_;
    }

    Steps const& steps() const {
        return steps_;
    }

private:
    Steps steps_;

public:
    /**
     * Output a diagnostic in a way that is human readable.
     *
     * Example output:
     *  <begin output>
     *  thread 1 acquired A, X, B
     *  while
     *  thread 2 acquired B, X, A
     *  which creates a deadlock if
     *      thread 1 acquires A and waits for B
     *      thread 2 acquires B and waits for A
     *  <end output>
     */
    friend std::ostream& operator<<(std::ostream&, DeadlockDiagnostic const&);
};
} // end namespace deadlock_diagnostic_detail

namespace core {
    using deadlock_diagnostic_detail::AcquireStreak;
    using deadlock_diagnostic_detail::DeadlockDiagnostic;
}
} // end namespace d2

#endif // !D2_CORE_DEADLOCK_DIAGNOSTIC_HPP
