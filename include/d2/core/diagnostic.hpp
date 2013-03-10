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
#include <vector>


namespace d2 {
namespace diagnostic_detail {
//! Class representing the state of a single deadlocked thread.
struct deadlocked_thread : boost::partially_ordered<deadlocked_thread> {
    /**
     * Create a `deadlocked_thread` with the thread identifier `tid`,
     * holding a sequence of `locks` and waiting for the `waiting_for` lock.
     */
    template <typename Container>
    deadlocked_thread(ThreadId tid, BOOST_FWD_REF(Container) locks,
                                    LockId waiting_for)
        : tid(tid), holding(boost::forward<Container>(locks)),
          waiting_for(waiting_for)
    {
        BOOST_ASSERT_MSG(holding.size() >= 1,
            "a thread can't be deadlocked if it is not holding at least one "
            "lock while waiting for another one");
    }

    /**
     * Create a `deadlocked_thread` with the thread identifier `tid`,
     * holding a sequence of `locks` in the range delimited by [first, last)
     * and waiting for the `waiting_for` lock.
     */
    template <typename Iterator>
    deadlocked_thread(ThreadId tid, BOOST_FWD_REF(Iterator) first,
                      BOOST_FWD_REF(Iterator) last, LockId waiting_for)
        : tid(tid), holding(boost::forward<Iterator>(first),
                            boost::forward<Iterator>(last)),
          waiting_for(waiting_for)
    {
        BOOST_ASSERT_MSG(holding.size() >= 1,
            "a thread can't be deadlocked if it is not holding at least one "
            "lock while waiting for another one");
    }

    //! Thread identifier of the deadlocked thread.
    ThreadId tid;

    //! Type of the sequence of locks held by the thread.
    typedef std::vector<LockId> lock_sequence;

    /**
     * Collection of locks held by that thread at the moment of the deadlock.
     * The locks are ordered in their order of acquisition.
     */
    lock_sequence holding;

    //! Identifier of the lock the thread is waiting after.
    LockId waiting_for;

    /**
     * Return whether two `deadlocked_thread`s represent the same thread
     * holding the same sequence of locks in the same order and waiting for
     * the same lock.
     */
    friend bool
    operator==(deadlocked_thread const& a, deadlocked_thread const& b) {
        return a.tid == b.tid &&
               a.waiting_for == b.waiting_for &&
               a.holding == b.holding;
    }

    /**
     * Return whether `a`'s thread identifier is smaller than `b`'s.
     *
     * If both have the same thread identifier, the lock they are waiting
     * for is compared. If these are the same, their sequences of held locks
     * are compared lexicographically.
     *
     * @note While it is not generally useful to compare `deadlocked_thread`s
     *       together, it can be useful to store them in containers.
     */
    friend bool
    operator<(deadlocked_thread const& a, deadlocked_thread const& b) {
        if (a.tid < b.tid)
            return true;
        else if (a.tid == b.tid)
            return a.waiting_for < b.waiting_for ||
                   (a.waiting_for == b.waiting_for && a.holding < b.holding);
        else
            return false;
    }

    //! Print a human readable representation of a `deadlocked_thread`.
    D2_DECL friend
    std::ostream& operator<<(std::ostream&, deadlocked_thread const&);
};

/**
 * Type representing a state which, if reached, would create a
 * deadlock in the program.
 *
 * A thread identifier is guaranteed to appear at most once in the sequence
 * of threads. Also, it is guaranteed that at least two threads are present
 * in the sequence, otherwise a deadlock is impossible.
 */
class potential_deadlock : boost::less_than_comparable<potential_deadlock> {
    void invariants() const {
        BOOST_ASSERT_MSG(threads.size() >= 2,
            "a deadlock can't happen with fewer than 2 threads");
        BOOST_ASSERT_MSG(!has_duplicate_threads(),
            "it makes no sense for the same thread to appear more than "
            "once in the sequence of deadlocked threads");
    }

    /**
     * Return whether the sequence of threads contains more than one thread
     * with the same identifier.
     */
    D2_DECL bool has_duplicate_threads() const;

public:
    /**
     * Create a `potential_deadlock` from a sequence of `deadlocked_thread`s
     * delimited by [first, last).
     *
     * @pre A thread identifier appears at most once in the sequence.
     * @pre There are at least two threads in the sequence, otherwise a
     *      deadlock is impossible.
     */
    template <typename Iterator>
    potential_deadlock(BOOST_FWD_REF(Iterator) first,
                       BOOST_FWD_REF(Iterator) last)
        : threads(boost::forward<Iterator>(first),
                  boost::forward<Iterator>(last))
    {
        invariants();
    }

    /**
     * Create a `potential_deadlock` from the `deadlocked_thread`s contained
     * in `threads`.
     *
     * @pre A thread identifier appears at most once in the sequence.
     * @pre There are at least two threads in the sequence, otherwise a
     *      deadlock is impossible.
     */
    template <typename Container>
    explicit potential_deadlock(BOOST_FWD_REF(Container) threads)
        : threads(boost::forward<Container>(threads))
    {
        invariants();
    }

    /**
     * Return the lexicographical comparison of the sequences of threads of
     * two `potential_deadlock`s.
     *
     * @note This is mostly (if not only) useful to store
     *       `potential_deadlock`s in ordered containers.
     */
    friend bool
    operator<(potential_deadlock const& a, potential_deadlock const& b) {
        return a.threads < b.threads;
    }

    /**
     * Return whether a deadlock is equivalent to another, i.e. if it consists
     * of the same sequence of threads in a possibly different order.
     */
    D2_DECL bool is_equivalent_to(potential_deadlock const& other) const;

    //! Type of the container holding the sequence of `deadlocked_thread`s.
    typedef std::vector<deadlocked_thread> thread_sequence;

    //! The actual sequence of threads involved in the deadlock.
    thread_sequence threads;
};

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
