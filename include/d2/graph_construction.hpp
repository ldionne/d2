/**
 * This file defines the algorithm to build the graphs from a program trace.
 */

#ifndef D2_GRAPH_CONSTRUCTION_HPP
#define D2_GRAPH_CONSTRUCTION_HPP

#include <d2/events.hpp>
#include <d2/graphs.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/algorithm/cxx11/any_of.hpp>
#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/static_assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/variant.hpp>
#include <cstddef>
#include <stdexcept>
#include <utility>


namespace d2 {
namespace detail {

template <typename Iterator, typename LockGraph, typename SegmentationGraph>
class GraphBuilder {
    // Label present on each edge of the lock graph. It contains
    // arbitrary information.
    typedef typename boost::edge_property_type<LockGraph>::type
                                                        LockGraphEdgeLabel;

    // Mapping of a thread to the segment in which it currently is.
    typedef boost::unordered_map<Thread, Segment> SegmentationContext;

    /**
     * Represents a lock that is currently held by a thread. The segment in
     * which the lock was acquired is recorded. Other arbitrary data can also
     * be recorded, which is useful to provide more information about the
     * locks, such as the call stack.
     */
    struct CurrentlyHeldLock : boost::equality_comparable<CurrentlyHeldLock> {
        SyncObject lock;
        Segment segment;
        LockDebugInfo info;

        CurrentlyHeldLock(SyncObject const& l, Segment const& s,
                          LockDebugInfo const& i)
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

    // Mapping of a thread to the set of locks currently held by it.
    typedef boost::unordered_map<Thread, HeldLocks> LockContext;

    // Given the first event, deduce the main thread.
    struct MainThreadDeducer : boost::static_visitor<Thread> {
        Thread operator()(StartEvent const& first) const {
            return first.parent;
        }
        Thread operator()(AcquireEvent const& first) const {
            return first.thread;
        }
        template <typename T>
        Thread operator()(T const&) const {
            boost::throw_exception(
                std::runtime_error(
                    "first event is not a start or an acquire event"));
            return *static_cast<Thread*>(NULL); // Never reached.
        }
    };

    template <typename T, typename Container>
    static bool contains(T const& val, Container const& c) {
        return c.find(val) != boost::end(c);
    }

    static bool exists_edge_between(SyncObject const& u_,
                                    SyncObject const& v_,
                                    LockGraphEdgeLabel const& label,
                                    LockGraph const& lg) {
        typedef typename boost::graph_traits<
                                LockGraph>::edge_descriptor EdgeDescriptor;
        typedef typename boost::graph_traits<
                            LockGraph>::vertex_descriptor VertexDescriptor;

        boost::optional<VertexDescriptor> u = find_vertex(u_, lg),
                                          v = find_vertex(v_, lg);
        BOOST_ASSERT_MSG(u && v,
            "trying to find an edge between two synchronization objects of "
            "which at least one has no associated vertex in the lock graph.");
        BOOST_FOREACH(EdgeDescriptor e, out_edges(*u, lg)) {
            if (target(e, lg) == *v && lg[e] == label)
                return true;
        }
        return false;
    }

    struct EventVisitor : boost::static_visitor<void> {
        LockGraph& lg;
        SegmentationGraph& sg;
        SegmentationContext& segment_of;
        LockContext& locks_held_by;
        Segment& n;

        EventVisitor(LockGraph& lg_, SegmentationGraph& sg_,
                     SegmentationContext& sc, LockContext& lc, Segment& s)
            : lg(lg_), sg(sg_), segment_of(sc), locks_held_by(lc), n(s)
        { }

        void operator()(AcquireEvent const& e) {
            Thread t(e.thread);
            BOOST_ASSERT_MSG(contains(t, segment_of),
                "acquiring a lock in a thread that has not been started yet");
            Segment s2(segment_of[t]);
            SyncObject l2(e.lock);

            // Each lock has only one vertex in the lock graph. Normally, we
            // should add a vertex only if a vertex representing the newly
            // acquired lock is not present in the graph. However, since the
            // lock_graph is a named_graph, this is already handled when we
            // use the lock_graph's vertex_name to add the vertex instead of
            // its vertex_descriptor.
            add_vertex(l2, lg);

            // Compute the gatelock set, i.e. the set of locks currently
            // held by `t`.
            HeldLocks& locks_held_by_t = locks_held_by[t];
            boost::unordered_set<SyncObject> g;
            BOOST_FOREACH(CurrentlyHeldLock const& l, locks_held_by_t)
                g.insert(l.lock);

            // Add an edge from every lock l1 already held by `t` to l2.
            BOOST_FOREACH(CurrentlyHeldLock const& l, locks_held_by_t) {
                SyncObject l1(l.lock);
                Segment s1(l.segment);
                LockGraphEdgeLabel label(l.info, s1, t, g, s2, e.info);
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
                if (!exists_edge_between(l1, l2, label, lg))
                    add_edge(l1, l2, label, lg);
            }
            locks_held_by_t.insert(CurrentlyHeldLock(l2, s2, e.info));
        }

        void operator()(ReleaseEvent const& e) {
            Thread t(e.thread);
            SyncObject l(e.lock);

            HeldLocks& context = locks_held_by[t];
            BOOST_ASSERT_MSG(boost::algorithm::any_of(
                                boost::begin(context), boost::end(context),
                        &boost::lambda::_1->*&CurrentlyHeldLock::lock == l),
                            "thread releasing a lock that it is not holding");

            //Release the lock; remove all locks equal to it from the context.
            {
                typedef typename HeldLocks::const_iterator Iter;
                Iter it(boost::begin(context)), last(boost::end(context));
                while (it != last)
                    it->lock == l ? (void)context.erase(it++) : (void)++it;
            }
        }

        void operator()(StartEvent const& e) {
            Thread parent(e.parent), child(e.child);
            BOOST_ASSERT_MSG(parent != child, "thread starting itself");
            BOOST_ASSERT_MSG(contains(parent, segment_of),
       "starting a thread from another thread that has not been created yet");

            // Segments:      parent    n+1    n+2
            // Parent thread:   o________o
            // Child thread:     \______________o
            Segment n_plus_1 = add_vertex(sg), n_plus_2 = add_vertex(sg);
            add_edge(segment_of[parent], n_plus_1, sg);
            add_edge(segment_of[parent], n_plus_2, sg);
            segment_of[parent] = n_plus_1;
            segment_of[child] = n_plus_2;
            n = n_plus_2;
        }

        void operator()(JoinEvent const& e) {
            Thread parent(e.parent), child(e.child);
            BOOST_ASSERT_MSG(parent != child, "thread joining itself");
            BOOST_ASSERT_MSG(contains(parent, segment_of),
        "joining a thread into another thread that has not been created yet");
            BOOST_ASSERT_MSG(contains(child, segment_of),
                            "joining a thread that has not been created yet");

            // Note: Below, the (parent, child, n) segments are not
            //       necessarily ordered that way. `Any thread` can refer
            //       to any thread, including `parent` or `child`.
            // Segments:      parent    child    n    n+1
            // Parent thread:   o______________________o
            // Child thread:              o___________/
            // Any thread:                       o
            Segment n_plus_1 = add_vertex(sg);
            add_edge(segment_of[parent], n_plus_1, sg);
            add_edge(segment_of[child], n_plus_1, sg);
            segment_of[parent] = n_plus_1;
            segment_of.erase(child);
            n = n_plus_1;
        }
    };

public:
    void operator()(Iterator first, Iterator last,
                                LockGraph& lg, SegmentationGraph& sg) const {
        if (first == last)
            return;

        SegmentationContext segment_of;
        LockContext locks_held_by;
        Segment n = add_vertex(sg); // Last taken segment.

        // The main thread is deduced using the first event.
        Thread main_thread(boost::apply_visitor(MainThreadDeducer(), *first));

        // Main thread implicitly starts in the first segment.
        segment_of[main_thread] = n;

        EventVisitor visitor(lg, sg, segment_of, locks_held_by, n);
        for (; first != last; ++first)
            boost::apply_visitor(visitor, *first);
    }
};
} // end namespace detail

/**
 * Build a lock graph and a segmentation graph from the events contained in
 * the range delimited by [first, last).
 */
template <typename Iterator, typename LockGraph, typename SegmentationGraph>
void build_graphs(Iterator first, Iterator last,
                                    LockGraph& lg, SegmentationGraph& sg) {
    // We must be able to add new vertices/edges and to set their
    // respective properties to build the lock graph.
    BOOST_CONCEPT_ASSERT((
                    boost::concepts::VertexMutablePropertyGraph<LockGraph>));
    BOOST_CONCEPT_ASSERT((
                    boost::concepts::EdgeMutablePropertyGraph<LockGraph>));

    // We need to be able to add new vertices/edges to build the
    // segmentation graph.
    BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<SegmentationGraph>));

    BOOST_CONCEPT_ASSERT((LockGraphConcept<LockGraph>));
    BOOST_CONCEPT_ASSERT((boost::InputIteratorConcept<Iterator>));
    BOOST_STATIC_ASSERT((::boost::is_same<
                            typename boost::iterator_value<Iterator>::type,
                            Event
                        >::value));

    detail::GraphBuilder<Iterator, LockGraph, SegmentationGraph> builder;
    builder(first, last, lg, sg);
}

template <typename Range, typename LockGraph, typename SegmentationGraph>
void build_graphs(Range const& range, LockGraph& lg, SegmentationGraph& sg) {
    build_graphs(boost::begin(range), boost::end(range), lg, sg);
}

} // end namespace d2

#endif // !D2_GRAPH_CONSTRUCTION_HPP
