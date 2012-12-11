/**
 * This file defines the algorithm to build the graphs from a program trace.
 */

#ifndef D2_GRAPH_CONSTRUCTION_HPP
#define D2_GRAPH_CONSTRUCTION_HPP

#include <d2/events.hpp>
#include <d2/types.hpp>

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
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator_range.hpp>
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
class graph_builder {
    // Label present on each edge of the lock graph. It contains
    // arbitrary information.
    typedef typename boost::edge_property_type<LockGraph>::type
                                                        LockGraphEdgeLabel;

    // Mapping of a thread to the segment in which it currently is.
    typedef boost::unordered_map<thread, segment> SegmentationContext;

    /**
     * Represents a lock that is currently held by a thread. The segment in
     * which the lock was acquired is recorded. Other arbitrary data can also
     * be recorded, which is useful to provide more information about the
     * locks, such as the call stack.
     */
    struct CurrentlyHeldLock : boost::equality_comparable<CurrentlyHeldLock> {
        sync_object lock;
        segment seg;
        detail::lock_debug_info info;

        CurrentlyHeldLock(sync_object const& l, segment const& s,
                          detail::lock_debug_info const& i)
            : lock(l), seg(s), info(i)
        { }

        friend bool operator==(CurrentlyHeldLock const& a,
                               CurrentlyHeldLock const& b) {
            return a.lock == b.lock && a.seg == b.seg;
        }

        friend std::size_t hash_value(CurrentlyHeldLock const& self) {
            std::size_t seed = 0;
            boost::hash_combine(seed, self.lock);
            boost::hash_combine(seed, self.seg);
            return seed;
        }
    };

    // Set of locks held by a thread at any given time.
    typedef boost::unordered_set<CurrentlyHeldLock> HeldLocks;

    // Mapping of a thread to the set of locks currently held by it.
    typedef boost::unordered_map<thread, HeldLocks> LockContext;

    // Given the first event, deduce the main thread.
    struct MainThreadDeducer : boost::static_visitor<thread> {
        thread operator()(start_event const& first) const {
            return first.parent;
        }
        thread operator()(acquire_event const& first) const {
            return first.thread;
        }
        template <typename T>
        thread operator()(T const&) const {
            boost::throw_exception(
                std::runtime_error(
                    "first event is not a start or an acquire event"));
            return *static_cast<thread*>(NULL); // Never reached.
        }
    };

    template <typename T, typename Container>
    static bool contains(T const& val, Container const& c) {
        return c.find(val) != boost::end(c);
    }

    struct EventVisitor : boost::static_visitor<void> {
        LockGraph& lg;
        SegmentationGraph& sg;
        SegmentationContext& segment_of;
        LockContext& locks_held_by;
        segment& n;

        EventVisitor(LockGraph& lg_, SegmentationGraph& sg_,
                     SegmentationContext& sc, LockContext& lc, segment& s)
            : lg(lg_), sg(sg_), segment_of(sc), locks_held_by(lc), n(s)
        { }

        void operator()(acquire_event const& e) {
            thread t(e.thread);
            BOOST_ASSERT_MSG(contains(t, segment_of),
                "acquiring a lock in a thread that has not been started yet");
            segment s2(segment_of[t]);
            sync_object l2(e.lock);

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
            boost::unordered_set<sync_object> g;
            BOOST_FOREACH(CurrentlyHeldLock const& l, locks_held_by_t)
                g.insert(l.lock);

            // Add an edge from every lock l1 already held by `t` to l2.
            BOOST_FOREACH(CurrentlyHeldLock const& l, locks_held_by_t) {
                sync_object l1(l.lock);
                segment s1(l.seg);
                LockGraphEdgeLabel label = {l.info, s1, t, g, s2, e.info};
                add_edge(l1, l2, label, lg);
            }
            locks_held_by_t.insert(CurrentlyHeldLock(l2, s2, e.info));
        }

        void operator()(release_event const& e) {
            thread t(e.thread);
            sync_object l(e.lock);

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

        void operator()(start_event const& e) {
            thread parent(e.parent), child(e.child);
            BOOST_ASSERT_MSG(parent != child, "thread starting itself");
            BOOST_ASSERT_MSG(contains(parent, segment_of),
       "starting a thread from another thread that has not been created yet");

            // Segments:      parent    n+1    n+2
            // Parent thread:   o________o
            // Child thread:     \______________o
            segment n_plus_1 = add_vertex(sg), n_plus_2 = add_vertex(sg);
            add_edge(segment_of[parent], n_plus_1, sg);
            add_edge(segment_of[parent], n_plus_2, sg);
            segment_of[parent] = n_plus_1;
            segment_of[child] = n_plus_2;
            n = n_plus_2;
        }

        void operator()(join_event const& e) {
            thread parent(e.parent), child(e.child);
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
            segment n_plus_1 = add_vertex(sg);
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
        segment n = add_vertex(sg); // Last taken segment.

        // The main thread is deduced using the first event.
        thread main_thread(boost::apply_visitor(MainThreadDeducer(), *first));

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
                            event
                        >::value));

    detail::graph_builder<Iterator, LockGraph, SegmentationGraph> builder;
    builder(first, last, lg, sg);
}

template <typename Iterator, typename LockGraph, typename SegmentationGraph>
void build_graphs(boost::iterator_range<Iterator> range,
                                    LockGraph& lg, SegmentationGraph& sg) {
    build_graphs(boost::begin(range), boost::end(range), lg, sg);
}

} // end namespace d2

#endif // !D2_GRAPH_CONSTRUCTION_HPP
