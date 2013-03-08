/**
 * This file defines utilities to provide diagnostics to the user from the
 * result of the analysis.
 */

#ifndef D2_CORE_DIAGNOSTIC_HPP
#define D2_CORE_DIAGNOSTIC_HPP

#include <d2/detail/decl.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <boost/assert.hpp>
#include <boost/move/utility.hpp>
#include <boost/operators.hpp>
#include <iosfwd>
#include <set>


namespace d2 {
namespace diagnostic_detail {
//! Class representing the state of a single deadlocked thread.
struct deadlocked_thread : boost::partially_ordered<deadlocked_thread> {
    deadlocked_thread(ThreadId tid, BOOST_RV_REF(std::vector<LockId>) locks)
        : tid(tid), locks(boost::move(locks))
    { }

    deadlocked_thread(ThreadId tid, std::vector<LockId> const& locks)
        : tid(tid), locks(locks)
    { }

    //! Thread identifier of the deadlocked thread.
    ThreadId tid;

    /**
     * Collection of locks held by that thread at the moment of the deadlock.
     * The locks are ordered in their order of acquisition.
     */
    std::vector<LockId> locks;

    /**
     * Return whether two `deadlocked_thread`s represent the same thread
     * holding the same set of locks in the same order.
     */
    friend bool
    operator==(deadlocked_thread const& a, deadlocked_thread const& b) {
        return a.tid == b.tid && a.locks == b.locks;
    }

    /**
     * Return whether `a`'s identifier is smaller than `b`'s. If both
     * have the same identifier, the operator returns whether `a`'s
     * locks are lexicographically smaller than `b`'s.
     *
     * @note While it is not generally useful to compare `deadlocked_thread`s
     *       together, this operator is useful for determining the equivalence
     *       of two `deadlocked_thread`s.
     */
    friend bool
    operator<(deadlocked_thread const& a, deadlocked_thread const& b) {
        return a.tid < b.tid || (a.tid == b.tid && a.locks < b.locks);
    }

    //! Print a human readable representation of a `deadlocked_thread`.
    D2_DECL friend
    std::ostream& operator<<(std::ostream&, deadlocked_thread const&);
};

/**
 * Predicate ensuring the partial order required by `potential_deadlock`.
 *
 * @internal We also order the threads from smallest thread identifier to
 *           largest to make the result deterministic, but this is an
 *           implementation detail.
 *
 * @see `potential_deadlock`
 */
struct chain_predicate {
    typedef bool result_type;
    result_type operator()(deadlocked_thread const& a,
                           deadlocked_thread const& b) const {
        BOOST_ASSERT_MSG(a.locks.size() >= 2 && b.locks.size() >= 2,
            "can't have a deadlocked thread with fewer than 2 locks taken");

        if (a.locks.back() == b.locks.front()) {
            if (a.locks.front() != b.locks.back())
                return true;
            else
                return a.tid < b.tid;
        }
        else
            return false;
    }
};

/**
 * Type representing a state which, if reached, would create a
 * deadlock in the program.
 *
 * The steps leading each thread to its deadlock state are guaranteed to be
 * ordered relatively to each other in a way that creates a chain.
 *
 * Example of a correct partial order:
 *  Steps of thread 1: A, B
 *  Steps of thread 2: B, C
 *  Steps of thread 3: C, A
 *
 * This creates the `chain' [A, (B], [C), A]. Any other configuration
 * yielding a valid chain is considered valid. Anything else is not.
 */
typedef std::set<deadlocked_thread, chain_predicate> potential_deadlock;

/**
 * Write an explanation of the potential deadlock state in plain text to
 * an output stream.
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
D2_DECL extern std::ostream&
plain_text_explanation(std::ostream&, potential_deadlock const&);
} // end namespace diagnostic_detail

namespace core {
    using diagnostic_detail::deadlocked_thread;
    using diagnostic_detail::plain_text_explanation;
    using diagnostic_detail::potential_deadlock;
}
} // end namespace d2

#endif // !D2_CORE_DIAGNOSTIC_HPP
