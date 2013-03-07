/**
 * This file defines utilities to provide diagnostics to the user from the
 * result of the analysis.
 */

#ifndef D2_CORE_DIAGNOSTIC_HPP
#define D2_CORE_DIAGNOSTIC_HPP

#include <d2/detail/decl.hpp>
#include <d2/lock_id.hpp>
#include <d2/thread_id.hpp>

#include <boost/move/utility.hpp>
#include <boost/operators.hpp>
#include <iosfwd>
#include <vector>


namespace d2 {
namespace diagnostic_detail {
//! Class representing the state of a single deadlocked thread.
struct deadlocked_thread : boost::equality_comparable<deadlocked_thread> {
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
};

/**
 * Type representing a state which, if reached, would create a
 * deadlock in the program.
 */
typedef std::vector<deadlocked_thread> potential_deadlock;

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
