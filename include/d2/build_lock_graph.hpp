/**
 * This file defines an algorithm to construct a directed graph describing
 * the relationship between the locks and the threads of a program.
 */

#ifndef D2_BUILD_LOCK_GRAPH_HPP
#define D2_BUILD_LOCK_GRAPH_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/events/acquire_event.hpp>
#include <d2/events/exceptions.hpp>
#include <d2/events/recursive_acquire_event.hpp>
#include <d2/events/recursive_release_event.hpp>
#include <d2/events/release_event.hpp>
#include <d2/events/segment_hop_event.hpp>
#include <d2/lock_graph.hpp>
#include <d2/lock_id.hpp>
#include <d2/segment.hpp>
#include <d2/thread_id.hpp>

#include <boost/assert.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/integer_traits.hpp>
#include <boost/move/move.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cstddef>
#include <typeinfo>
#include <utility>


namespace d2 {

namespace detail {
/**
 * Return whether `v` is adjacent to `u` using an edge with the given property.
 */
template <typename Graph, typename EdgeProperty>
bool is_adjacent(Graph const& graph,
                 typename boost::graph_traits<Graph>::vertex_descriptor u,
                 typename boost::graph_traits<Graph>::vertex_descriptor v,
                 EdgeProperty const& property) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    BOOST_FOREACH(Edge e, out_edges(u, graph))
        if (target(e, graph) == v && graph[e] == property)
            return true;
    return false;
}

template <typename Graph, typename EdgeProperty>
bool is_adjacent(Graph const& graph,
                 typename boost::graph_traits<Graph>::vertex_descriptor u,
                 typename Graph::vertex_name_type const& v_,
                 EdgeProperty const& property) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    boost::optional<Vertex> v = find_vertex(v_, graph);
    BOOST_ASSERT_MSG(v, "trying to find an edge between two vertices using "
        "their vertex name but the destination vertex name has no associated "
        "vertex in the graph");
    return is_adjacent(graph, u, *v, property);
}

template <typename Graph, typename VertexNameOrDesc, typename EdgeProperty>
bool is_adjacent(Graph const& graph,
                 typename Graph::vertex_name_type const& u_,
                 VertexNameOrDesc const& v_,
                 EdgeProperty const& property) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    boost::optional<Vertex> u = find_vertex(u_, graph);
    BOOST_ASSERT_MSG(u, "trying to find an edge between two vertices using "
        "their vertex name but the source vertex name has no associated "
        "vertex in the graph");
    return is_adjacent(graph, *u, v_, property);
}
} // end namespace detail

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

        CurrentlyHeldLock(LockId const& lock, Segment const& segment,
                          detail::LockDebugInfo const& info)
            : lock(lock), segment(segment), info(info)
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

    template <typename Graph>
    struct EventVisitor : boost::static_visitor<void> {
        Graph& graph;
        HeldLocks& held_locks;
        ThreadId& this_thread;
        Segment current_segment;
        boost::unordered_map<LockId, std::size_t> recursive_lock_count;

        typedef typename boost::edge_property_type<Graph>::type EdgeLabel;
        typedef boost::graph_traits<Graph> Traits;
        typedef typename Traits::vertex_descriptor VertexDescriptor;
        typedef typename Traits::edge_descriptor EdgeDescriptor;

        // Note:
        // There are two possible cases for the current_segment:
        //      - this_thread is not the main thread, the first event is a
        //          SegmentHopEvent, and the current_segment will be set to
        //          the correct value on the first application of the visitor.
        //      - this_thread is the main thread, the first event is NOT a
        //          SegmentHopEvent, and the current_segment is set to its
        //          initial value (default constructed) until we encounter a
        //          SegmentHopEvent.
        EventVisitor(Graph& lg, HeldLocks& hl, ThreadId& this_thread)
            : graph(lg), held_locks(hl), this_thread(this_thread)
        { }

        template <typename Event>
        void operator()(Event const& event) {
            if (!SilentlyIgnoreOtherEvents)
                D2_THROW(EventTypeException()
            << ExpectedType("SegmentHopEvent, AcquireEvent or ReleaseEvent")
            << ActualType(typeid(event).name()));
        }

        void operator()(SegmentHopEvent const& event) {
            ThreadId thread(thread_of(event));
            if (thread != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread));
            current_segment = segment_of(event);
        }

        void operator()(AcquireEvent const& event) {
            ThreadId t(thread_of(event));
            if (t != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(t));
            Segment s2(current_segment);
            LockId l2(lock_of(event));

            // Each lock has only one vertex in the lock graph. Normally, we
            // should add a vertex only if a vertex representing the newly
            // acquired lock is not present in the graph. However, since the
            // lock_graph is a named_graph, this is already handled when we
            // use the lock_graph's vertex_name to add the vertex instead of
            // its vertex_descriptor.
            VertexDescriptor l2_vertex = add_vertex(l2, graph);

            // Compute the gatelock set, i.e. the set of locks currently
            // held by this thread.
            detail::Gatelocks::underlying_set_type g_tmp;
            BOOST_FOREACH(CurrentlyHeldLock const& l, held_locks)
                g_tmp.insert(l.lock);
            detail::Gatelocks g(boost::move(g_tmp));

            // Add an edge from every lock l1 already held by
            // this thread to l2.
            BOOST_FOREACH(CurrentlyHeldLock const& l, held_locks) {
                LockId l1(l.lock);
                Segment s1(l.segment);
                EdgeLabel label(l.info, s1, t, g, s2, event.info);

                boost::optional<VertexDescriptor>
                    l1_vertex_maybe = find_vertex(l1, graph);
                BOOST_ASSERT_MSG(l1_vertex_maybe, "a lock this thread is "
                    "holding has no associated vertex in the lock graph");
                VertexDescriptor l1_vertex = *l1_vertex_maybe;

                // We don't add an edge if there is already an edge that is
                // exactly the same, since this would only create redundancy
                // in the graph. Multiple equal parallel edges would happen
                // if some synchronization objects were acquired and released
                // in a loop. Right now, the additionnal information carried
                // on each edge is considered to determine the uniqueness of
                // an edge, because if the acquisition happened at a different
                // place in the code, we would still want to detect a
                // different deadlock during the analysis. See the
                // simple_ABBA_redudant_diff_functions test for more info.
                if (!detail::is_adjacent(graph, l1_vertex, l2_vertex, label)) {
                    std::pair<EdgeDescriptor, bool> added_edge =
                                add_edge(l1_vertex, l2_vertex, label, graph);

                    BOOST_ASSERT_MSG(added_edge.second,
                        "two vertices that are not adjacent via an edge with "
                        "the current label could not be connected");
                }
            }
            held_locks.insert(CurrentlyHeldLock(l2, s2, event.info));
        }

        void operator()(RecursiveAcquireEvent const& event) {
            ThreadId thread(thread_of(event));
            LockId lock(lock_of(event));
            if (thread != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread));

            std::size_t& lock_count = recursive_lock_count[lock];
            // This is very unlikely, but it *could* happen and we *must*
            // handle it gracefully.
            if (lock_count == ::boost::integer_traits<std::size_t>::const_max)
                D2_THROW(RecursiveLockOverflowException()
                            << CurrentThread(this_thread)
                            << OverflowingLock(lock));
            // If this is the first time its being locked, then we must
            // signal an acquire event. In all cases, we increment the
            // number of times this lock has been locked.
            if (lock_count++ == 0) {
                AcquireEvent acquire_event(lock, thread);
                acquire_event.info = event.info;
                (*this)(acquire_event);
            }
        }

        void operator()(RecursiveReleaseEvent const& event) {
            ThreadId thread(thread_of(event));
            LockId lock(lock_of(event));
            if (thread != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread));

            std::size_t& lock_count = recursive_lock_count[lock];
            if (lock_count == 0)
                D2_THROW(UnexpectedReleaseException()
                            << ReleasingThread(this_thread)
                            << ReleasedLock(lock));

            // If this is a top level release, i.e. the thread does not hold
            // the lock at all anymore after this release, then we really
            // signal a release event.
            if (--lock_count == 0)
                (*this)(ReleaseEvent(lock, thread));
        }

        void operator()(ReleaseEvent const& event) {
            ThreadId thread(thread_of(event));
            LockId lock(lock_of(event));
            if (thread != this_thread)
                D2_THROW(EventThreadException()
                            << ExpectedThread(this_thread)
                            << ActualThread(thread));

            //Release the lock; remove all locks equal to it from the context.
            typename HeldLocks::const_iterator it(boost::begin(held_locks)),
                                               last(boost::end(held_locks));
            bool l_is_owned_by_this_thread = false;
            while (it != last) {
                if (it->lock == lock) {
                    held_locks.erase(it++);
                    l_is_owned_by_this_thread = true;
                }
                else
                    ++it;
            }
            if (!l_is_owned_by_this_thread)
                D2_THROW(UnexpectedReleaseException()
                            << ReleasingThread(this_thread)
                            << ReleasedLock(lock));
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

        ThreadId operator()(RecursiveAcquireEvent const& event) const
        { return thread_of(event); }

        ThreadId operator()(AcquireEvent const& event) const
        { return thread_of(event); }

        ThreadId operator()(SegmentHopEvent const& event) const
        { return thread_of(event); }
    };

public:
    typedef void result_type;

    template <typename Iterator, typename Graph>
    result_type operator()(Iterator first, Iterator last, Graph& graph) const {
        // We must be able to add new vertices/edges and to set their
        // respective properties to build the lock graph.
        // Note: See the note in build_segmentation_graph.hpp to know why
        //       these concept checks are disabled.
#if 0
        BOOST_CONCEPT_ASSERT((boost::VertexMutablePropertyGraphConcept<Graph>));
        BOOST_CONCEPT_ASSERT((boost::EdgeMutablePropertyGraphConcept<Graph>));
#endif
        BOOST_MPL_ASSERT((boost::is_multigraph<Graph>));
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

        EventVisitor<Graph> visitor(graph, held_locks, this_thread);
        for (; first != last; ++first)
            boost::apply_visitor(visitor, *first);
    }

    template <typename Range, typename Graph>
    result_type operator()(Range const& range, Graph& graph) const {
        return (*this)(boost::begin(range), boost::end(range), graph);
    }
};

} // end namespace d2

#endif // !D2_BUILD_LOCK_GRAPH_HPP
