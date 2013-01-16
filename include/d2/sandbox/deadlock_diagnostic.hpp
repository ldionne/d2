/**
 * This file defines classes to present deadlock potential diagnostics.
 */

#ifndef D2_SANDBOX_DEADLOCK_DIAGNOSTIC_HPP
#define D2_SANDBOX_DEADLOCK_DIAGNOSTIC_HPP

#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/optional.hpp>
#include <cstddef>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>


namespace d2 {
namespace sandbox {

/**
 * Class representing the diagnostic of a deadlock potential.
 *
 * @internal This class is not very optimized (sets created by some getters,
 *           etc...), but the truth is: we don't care! This is nowhere near
 *           a performance critical part and the number of deadlock potentials
 *           is likely to be small.
 */
class DeadlockDiagnostic {
    // Those are misnamed in the rest of d2, this should be refactored.
    typedef Thread ThreadId;
    typedef SyncObject LockId;

public:
    struct ThreadInformation {
        ThreadId thread_id;
        // the main thread has no parent
        boost::optional<ThreadInformation&> parent;
    };

    struct LockInformation {
        LockId lock_id;
        // Call stack, etc..
    };

    /**
     * Class representing a sequence of lock acquisitions by a single thread.
     */
    struct AcquireStreak {
        // Note: This solution is temporary until we find a better way to
        //       attach arbitrary data to graph edges.
        explicit AcquireStreak(ThreadId thread, LockId lock1, LockId lock2) {
            LockInformation l1 = {lock1}, l2 = {lock2};
            locks.push_back(l1);
            locks.push_back(l2);
            this->thread.thread_id = thread;
        }

    private:
        friend class DeadlockDiagnostic;
        ThreadInformation thread;
        // sorted in order of acquisition
        std::vector<LockInformation> locks;
    };

    template <typename Iterator>
    DeadlockDiagnostic(Iterator first_streak, Iterator last_streak)
        : steps_(first_streak, last_streak)
    { }

    std::size_t number_of_involved_threads() const;
    std::size_t number_of_involved_locks() const;
    std::set<ThreadInformation> involved_threads() const;
    std::set<LockInformation> involved_locks() const;

private:
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
    typedef std::vector<AcquireStreak> Steps;
    Steps steps_;

    static std::string format_step(AcquireStreak const& streak);
    static std::string format_explanation(AcquireStreak const& streak);

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

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_DEADLOCK_DIAGNOSTIC_HPP
