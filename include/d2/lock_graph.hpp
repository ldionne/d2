/**
 * This file defines the lock graph data structure used during the post-mortem
 * program analysis.
 */

#ifndef D2_LOCK_GRAPH_HPP
#define D2_LOCK_GRAPH_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/events/acquire_event.hpp>
#include <d2/events/exceptions.hpp>
#include <d2/events/recursive_acquire_event.hpp>
#include <d2/events/recursive_release_event.hpp>
#include <d2/events/release_event.hpp>
#include <d2/events/segment_hop_event.hpp>
#include <d2/lock_id.hpp>
#include <d2/segment.hpp>
#include <d2/thread_id.hpp>

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/assert.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/flyweight.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/integer_traits.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/variant.hpp>
#include <cstddef>
#include <set>
#include <typeinfo>


namespace d2 {

/**
 * Label present on the edges of a lock graph.
 */
struct LockGraphLabel : boost::equality_comparable<LockGraphLabel> {
    LockGraphLabel() { }

    LockGraphLabel(detail::LockDebugInfo const& l1_info, Segment s1,
                   ThreadId thread,
                   std::set<LockId> const& gatelocks,
                   Segment s2, detail::LockDebugInfo const& l2_info)
        : l1_info(l1_info), l2_info(l2_info), s1(s1), s2(s2),
          thread_(thread), gatelocks_(gatelocks)
    { }

    detail::LockDebugInfo l1_info, l2_info;
    Segment s1, s2;

    friend std::set<LockId> const& gatelocks_of(LockGraphLabel const& self) {
        return self.gatelocks_;
    }

    friend ThreadId const& thread_of(LockGraphLabel const& self) {
        return self.thread_;
    }

    friend bool operator==(LockGraphLabel const& a, LockGraphLabel const& b) {
        // Note: We test the easiest first, i.e. the threads and segments,
        //       which are susceptible of being similar to integers.
        return a.s1 == b.s1 &&
               a.s2 == b.s2 &&
               thread_of(a) == thread_of(b) &&
               a.l1_info == b.l1_info &&
               a.l2_info == b.l2_info &&
               gatelocks_of(a) == gatelocks_of(b);
    }

private:
    ThreadId thread_;
    boost::flyweight<std::set<LockId> > gatelocks_;
};

/**
 * Directed graph representing the contexts in which synchronization objects
 * were acquired by threads.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                            LockId, LockGraphLabel> LockGraph;

/**
 * Exception thrown when a lock is released and we were not expecting it.
 */
struct UnexpectedReleaseException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::UnexpectedReleaseException";
    }
};

/**
 * Exception thrown when an event comes from an unexpected thread.
 */
struct EventThreadException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::EventThreadException";
    }
};

/**
 * Exception thrown when a recursive lock is locked too many times for the
 * system to handle. While this is _very_ unlikely, we still handle it
 * gracefully.
 */
struct RecursiveLockOverflowException : virtual EventException {
    virtual char const* what() const throw() {
        return "d2::RecursiveLockOverflowException";
    }
};

namespace exception_tag {
    struct expected_thread;
    struct actual_thread;

    struct releasing_thread;
    struct released_lock;

    struct overflowing_lock;
    struct current_thread;
}

typedef boost::error_info<
            exception_tag::expected_thread, ThreadId
        > ExpectedThread;

typedef boost::error_info<
            exception_tag::actual_thread, ThreadId
        > ActualThread;

typedef boost::error_info<
            exception_tag::releasing_thread, ThreadId
        > ReleasingThread;

typedef boost::error_info<
            exception_tag::released_lock, LockId
        > ReleasedLock;

typedef boost::error_info<
            exception_tag::current_thread, ThreadId
        > CurrentThread;

typedef boost::error_info<
            exception_tag::overflowing_lock, LockId
        > OverflowingLock;

/**
 * Function object used to build the lock graph from a range of events.
 * The events should be `SegmentHopEvent`s, `AcquireEvent`s, and
 * `ReleaseEvent`s exclusively. Depending on the `SilentlyIgnoreOtherEvents`
 * template parameter, other events will be ignored or an
 * `UnexpectedEventException` will be thrown.
 *
 * Either way (`SilentlyIgnoreOtherEvents` or not), the functor provides
 * the basic exception guarantee.
 */
template <bool SilentlyIgnoreOtherEvents = true>
class build_lock_graph {
    /**
     * Represents a lock that is currently held by a thread. The segment in
     * which the lock was acquired is recorded. Other arbitrary data can also
     * be recorded, which is useful to provide more information about the
     * locks, such as the call stack.
     */
    struct CurrentlyHeldLock : boost::equality_comparable<CurrentlyHeldLock> {
        LockId lock;
        Segment segment;
        detail::LockDebugInfo info;

        CurrentlyHeldLock(LockId const& l, Segment const& s,
                          detail::LockDebugInfo const& i)
            : lock(l), segment(s), info(i)
        { }

        friend bool operator==(CurrentlyHeldLock const& a,
                               CurrentlyHeldLock const& b) {
            return a.lock == b.lock && a.segment == b.segment;
        }

        friend std::size_t hash_value(CurrentlyHeldLock const& self) {
            std::size_t seed = 0;
            boost::hash_combine(seed, self.lock);
            boost::hash_combine(seed, self.segment);
            return seed;
        }
    };

    // Set of locks held by a thread at any given time.
    typedef boost::unordered_set<CurrentlyHeldLock> HeldLocks;

    template <typename VertexName, typename EdgeProperty, typename Graph>
    static bool are_connected(VertexName const& u_, VertexName const& v_,
                              EdgeProperty const& prop, Graph const& graph) {
        typedef typename boost::graph_traits<
                                Graph>::edge_descriptor EdgeDescriptor;
        typedef typename boost::graph_traits<
                            Graph>::vertex_descriptor VertexDescriptor;

        boost::optional<VertexDescriptor> u = find_vertex(u_, graph),
                                          v = find_vertex(v_, graph);
        BOOST_ASSERT_MSG(u && v,
            "trying to find an edge between two synchronization objects of "
            "which at least one has no associated vertex in the lock graph.");
        BOOST_FOREACH(EdgeDescriptor e, out_edges(*u, graph))
            if (target(e, graph) == *v && graph[e] == prop)
                return true;
        return false;
    }

    template <typename LockGraph>
    struct EventVisitor : boost::static_visitor<void> {
        LockGraph& graph;
        HeldLocks& held_locks;
        ThreadId& this_thread;
        Segment current_segment;
        boost::unordered_map<LockId, std::size_t> recursive_lock_count;

        // Note:
        // There are two possible cases for the current_segment:
        //      - this_thread is not the main thread, the first event is a
        //          SegmentHopEvent, and the current_segment will be set to
        //          the correct value on the first application of the visitor.
        //      - this_thread is the main thread, the first event is NOT a
        //          SegmentHopEvent, and the current_segment is set to its
        //          initial value (default constructed) until we encounter a
        //          SegmentHopEvent.
        EventVisitor(LockGraph& lg, HeldLocks& hl, ThreadId& this_thread)
            : graph(lg), held_locks(hl), this_thread(this_thread)
        { }

        template <typename Event>
        void operator()(Event const& event) {
            if (!SilentlyIgnoreOtherEvents)
                D2_THROW(EventTypeException()
            << ExpectedType("SegmentHopEvent, AcquireEvent or ReleaseEvent")
            << ActualType(typeid(event).name()));
        }

        void operator()(SegmentHopEvent const& e) {
            if (thread_of(e) != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread_of(e)));
            current_segment = segment_of(e);
        }

        void operator()(AcquireEvent const& e) {
            if (thread_of(e) != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread_of(e)));
            ThreadId t(thread_of(e));
            Segment s2(current_segment);
            LockId l2(lock_of(e));

            // Each lock has only one vertex in the lock graph. Normally, we
            // should add a vertex only if a vertex representing the newly
            // acquired lock is not present in the graph. However, since the
            // lock_graph is a named_graph, this is already handled when we
            // use the lock_graph's vertex_name to add the vertex instead of
            // its vertex_descriptor.
            add_vertex(l2, graph);

            // Compute the gatelock set, i.e. the set of locks currently
            // held by this thread.
            std::set<LockId> g;
            BOOST_FOREACH(CurrentlyHeldLock const& l, held_locks)
                g.insert(l.lock);

            // Add an edge from every lock l1 already held by
            // this thread to l2.
            BOOST_FOREACH(CurrentlyHeldLock const& l, held_locks) {
                LockId l1(l.lock);
                Segment s1(l.segment);
                LockGraphLabel label(l.info, s1, t, g, s2, e.info);
                // We don't add an edge if there is already an edge that is
                // exactly the same, since this would only create redundancy
                // in the graph. Multiple equal parallel edges would happen
                // if some synchronization objects were acquired and released
                // in a loop. Right now, the additionnal information carried
                // on each edge is considered to determine the uniqueness of
                // an edge, because if the acquisition happened at a different
                // place in the code, we would still want to detect a
                // different deadlock during the analysis. See the
                // simple_ABBA_redudant_diff_functions test for an example.
                if (!are_connected(l1, l2, label, graph))
                    add_edge(l1, l2, label, graph);
            }
            held_locks.insert(CurrentlyHeldLock(l2, s2, e.info));
        }

        void operator()(RecursiveAcquireEvent const& e) {
            if (thread_of(e) != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread_of(e)));

            std::size_t& lock_count = recursive_lock_count[lock_of(e)];
            // This is very unlikely, but it *could* happen and we *must*
            // handle it gracefully.
            if (lock_count == ::boost::integer_traits<std::size_t>::const_max)
                D2_THROW(RecursiveLockOverflowException()
                            << CurrentThread(this_thread)
                            << OverflowingLock(lock_of(e)));
            // If this is the first time its being locked, then we must
            // signal an acquire event. In all cases, we increment the
            // number of times this lock has been locked.
            if (lock_count++ == 0) {
                AcquireEvent acquire_event(lock_of(e), thread_of(e));
                acquire_event.info = e.info;
                (*this)(acquire_event);
            }
        }

        void operator()(RecursiveReleaseEvent const& e) {
            if (thread_of(e) != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread_of(e)));

            std::size_t& lock_count = recursive_lock_count[lock_of(e)];
            if (lock_count == 0)
                D2_THROW(UnexpectedReleaseException()
                            << ReleasingThread(this_thread)
                            << ReleasedLock(lock_of(e)));

            // If this is a top level release, i.e. the thread does not hold
            // the lock at all anymore after this release, then we really
            // signal a release event.
            if (--lock_count == 0)
                (*this)(ReleaseEvent(lock_of(e), thread_of(e)));
        }

        void operator()(ReleaseEvent const& e) {
            if (thread_of(e) != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread_of(e)));

            ThreadId t(thread_of(e));
            LockId l(lock_of(e));

            //Release the lock; remove all locks equal to it from the context.
            typename HeldLocks::const_iterator it(boost::begin(held_locks)),
                                               last(boost::end(held_locks));
            bool l_is_owned_by_this_thread = false;
            while (it != last) {
                if (it->lock == l) {
                    held_locks.erase(it++);
                    l_is_owned_by_this_thread = true;
                }
                else
                    ++it;
            }
            if (!l_is_owned_by_this_thread)
                D2_THROW(UnexpectedReleaseException()
                            << ReleasingThread(this_thread)
                            << ReleasedLock(l));
        }
    };

    struct DeduceThisThread : boost::static_visitor<ThreadId> {
        template <typename Event>
        ThreadId operator()(Event const& event) const {
            D2_THROW(EventTypeException()
                        << ExpectedType("AcquireEvent or "
                                        "RecursiveAcquireEvent or "
                                        "SegmentHopEvent")
                        << ActualType(typeid(event).name()));
            return ThreadId(); // never reached.
        }

        ThreadId operator()(RecursiveAcquireEvent const& e) const
        { return thread_of(e); }

        ThreadId operator()(AcquireEvent const& e) const
        { return thread_of(e); }

        ThreadId operator()(SegmentHopEvent const& e) const
        { return thread_of(e); }
    };

public:
    typedef void result_type;

    template <typename Iterator, typename LockGraph>
    result_type
    operator()(Iterator first, Iterator last, LockGraph& graph) const {
        // We must be able to add new vertices/edges and to set their
        // respective properties to build the lock graph.
        // Note: See the note in segmentation_graph.hpp to know why these
        //       concept checks are disabled. It applies to LockGraph too.
#if 0
        BOOST_CONCEPT_ASSERT((
                    boost::concepts::VertexMutablePropertyGraph<LockGraph>));
        BOOST_CONCEPT_ASSERT((
                    boost::concepts::EdgeMutablePropertyGraph<LockGraph>));
#endif

        BOOST_CONCEPT_ASSERT((boost::InputIteratorConcept<Iterator>));

        if (first == last)
            return;

        // The set of locks currently held by the thread we're processing.
        // It obviously starts empty.
        HeldLocks held_locks;

        // The first event must be a SegmentHopEvent, because generating a
        // SegmentHopEvent is the first thing we do when a thread is started.
        // The only case where the first event is not a SegmentHopEvent is
        // for the main thread, in which case it can be an AcquireEvent too.
        // We deduce the thread we're processing from the first event.
        ThreadId this_thread=boost::apply_visitor(DeduceThisThread(), *first);

        EventVisitor<LockGraph> visitor(graph, held_locks, this_thread);
        for (; first != last; ++first)
            boost::apply_visitor(visitor, *first);
    }

    template <typename Range, typename LockGraph>
    result_type operator()(Range const& range, LockGraph& graph) const {
        return (*this)(boost::begin(range), boost::end(range), graph);
    }
};

} // end namespace d2

namespace boost {
namespace graph {

// This is to be able to refer to a vertex in the lock graph using the
// LockId associated to it.
template <> struct internal_vertex_name<d2::LockId> {
    typedef multi_index::identity<d2::LockId> type;
};

} // end namespace graph
} // end namespace boost

#endif // !D2_LOCK_GRAPH_HPP
