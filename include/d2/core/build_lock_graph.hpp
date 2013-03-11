/**
 * This file defines an algorithm to construct a directed graph describing
 * the relationship between the locks and the threads of a program.
 */

#ifndef D2_CORE_BUILD_LOCK_GRAPH_HPP
#define D2_CORE_BUILD_LOCK_GRAPH_HPP

#include <d2/core/events.hpp>
#include <d2/core/exceptions.hpp>
#include <d2/core/lock_graph.hpp>
#include <d2/detail/lock_debug_info.hpp>
#include <d2/lock_id.hpp>
#include <d2/segment.hpp>
#include <d2/thread_id.hpp>

#include <boost/assert.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/container/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/graph/properties.hpp>
#include <boost/integer_traits.hpp>
#include <boost/move/move.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix/core/argument.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/unordered_map.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cstddef>
#include <typeinfo>
#include <utility>


namespace d2 {
namespace build_lock_graph_detail {
/**
 * Return whether `v` is adjacent to `u` using an edge whose property
 * satisfies `predicate`.
 */
template <typename Graph, typename Predicate>
typename boost::disable_if<
    boost::is_same<Predicate, typename boost::edge_property_type<Graph>::type>,
bool>::type
is_adjacent(Graph const& graph,
            typename boost::graph_traits<Graph>::vertex_descriptor u,
            typename boost::graph_traits<Graph>::vertex_descriptor v,
            Predicate const& predicate) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    BOOST_FOREACH(Edge e, out_edges(u, graph))
        if (target(e, graph) == v && predicate(graph[e]))
            return true;
    return false;
}

template <typename Graph>
bool is_adjacent(Graph const& graph,
                 typename boost::graph_traits<Graph>::vertex_descriptor u,
                 typename boost::graph_traits<Graph>::vertex_descriptor v,
                 typename boost::edge_property_type<Graph>::type const& prop){
    return is_adjacent(graph, u, v, boost::phoenix::arg_names::_1 == prop);
}

template <typename Graph, typename Predicate>
bool is_adjacent(Graph const& graph,
                 typename boost::graph_traits<Graph>::vertex_descriptor u,
                 typename Graph::vertex_name_type const& v_,
                 Predicate const& predicate) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    boost::optional<Vertex> v = find_vertex(v_, graph);
    BOOST_ASSERT_MSG(v, "trying to find an edge between two vertices using "
        "their vertex name but the destination vertex name has no associated "
        "vertex in the graph");
    return is_adjacent(graph, u, *v, predicate);
}

template <typename Graph, typename VertexNameOrDesc, typename Predicate>
bool is_adjacent(Graph const& graph,
                 typename Graph::vertex_name_type const& u_,
                 VertexNameOrDesc const& v_,
                 Predicate const& predicate) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    boost::optional<Vertex> u = find_vertex(u_, graph);
    BOOST_ASSERT_MSG(u, "trying to find an edge between two vertices using "
        "their vertex name but the source vertex name has no associated "
        "vertex in the graph");
    return is_adjacent(graph, *u, v_, predicate);
}


/**
 * Represents a lock that is currently held by a thread. The segment in
 * which the lock was acquired is recorded along with some other arbitrary
 * data.
 */
template <typename ArbitraryData>
struct CurrentlyHeldLock
    : boost::equality_comparable<CurrentlyHeldLock<ArbitraryData> >
{
    LockId lock;
    Segment segment;
    ArbitraryData data;

    CurrentlyHeldLock(LockId const& lock, Segment const& segment,
                      ArbitraryData const& data)
        : lock(lock), segment(segment), data(data)
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

template <typename Graph, typename CustomVertexInfo, typename CustomEdgeInfo,
          bool SilentlyIgnoreOtherEvents>
struct EventVisitor : boost::static_visitor<void> {
    Graph& graph;
    ThreadId this_thread;

    typedef CurrentlyHeldLock<core::events::acquire::aux_info_type> HeldLock;

    // Set of locks currently held by the thread we're processing.
    typedef boost::container::vector<HeldLock> HeldLocks;
    HeldLocks held_locks;

    Segment current_segment;
    boost::unordered_map<LockId, std::size_t> recursive_lock_count;
    CustomVertexInfo custom_vertex_info;
    CustomEdgeInfo custom_edge_info;

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
    EventVisitor(Graph& lg, ThreadId const& this_thread,
                 CustomVertexInfo const& vertex_info,
                 CustomEdgeInfo const& edge_info)
        : graph(lg), this_thread(this_thread), held_locks(),
          current_segment(), recursive_lock_count(),
          custom_vertex_info(vertex_info), custom_edge_info(edge_info)
    { }

    template <typename Event>
    void operator()(Event const& event) {
        if (!SilentlyIgnoreOtherEvents)
            D2_THROW(EventTypeException()
        << ExpectedType("segment_hop, acquire or release")
        << ActualType(typeid(event).name()));
    }

    void operator()(core::events::segment_hop const& event) {
        ThreadId thread(get(core::events::tag::thread(), event));
        if (thread != this_thread)
            D2_THROW(EventThreadException()
                        << ExpectedThread(this_thread)
                        << ActualThread(thread));
        current_segment = segment_of(event);
    }

    template <typename Event>
    void process_acquire_event(Event const& event) {
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
        custom_vertex_info(l2_vertex, aux_info_of(event));

        // Compute the gatelock set, i.e. the set of locks currently
        // held by this thread.
        core::Gatelocks::underlying_set_type g_tmp;
        BOOST_FOREACH(HeldLock const& l, held_locks)
            g_tmp.get<1>().push_back(l.lock);
        core::Gatelocks g(boost::move(g_tmp));

        // Add an edge from every lock l1 already held by
        // this thread to l2.
        BOOST_FOREACH(HeldLock const& l, held_locks) {
            LockId l1(l.lock);
            Segment s1(l.segment);
            EdgeLabel label(s1, t, g, s2);
            // Temporary
            label.l1_info = l.data;
            label.l2_info = aux_info_of(event);

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
            if (!is_adjacent(graph, l1_vertex, l2_vertex, label)) {
                std::pair<EdgeDescriptor, bool> added_edge =
                            add_edge(l1_vertex, l2_vertex, label, graph);

                BOOST_ASSERT_MSG(added_edge.second,
                    "two vertices that are not adjacent via an edge with "
                    "the current label could not be connected");

                custom_edge_info(added_edge.first, l.data, aux_info_of(event));
            }
        }
        held_locks.push_back(HeldLock(l2, s2, aux_info_of(event)));
    }

    void operator()(core::events::acquire const& event) {
        process_acquire_event(event);
    }

    void operator()(core::events::recursive_acquire const& event) {
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
        if (lock_count++ == 0)
            process_acquire_event(event);
    }

    void operator()(core::events::recursive_release const& event) {
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
            (*this)(core::events::release(thread, lock));
    }

    void operator()(core::events::release const& event) {
        ThreadId thread(thread_of(event));
        LockId lock(lock_of(event));
        if (thread != this_thread)
            D2_THROW(EventThreadException()
                        << ExpectedThread(this_thread)
                        << ActualThread(thread));

        // Release the lock; remove all locks equal to it from the context.
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

template <typename ThreadId>
struct DeduceThisThread : boost::static_visitor<ThreadId> {
    template <typename Event>
    ThreadId operator()(Event const& event) const {
        D2_THROW(EventTypeException()
                    << ExpectedType("acquire, recursive_acquire or segment_hop")
                    << ActualType(typeid(event).name()));
        return ThreadId(); // never reached.
    }

    ThreadId operator()(core::events::recursive_acquire const& event) const
    { return thread_of(event); }

    ThreadId operator()(core::events::acquire const& event) const
    { return thread_of(event); }

    ThreadId operator()(core::events::segment_hop const& event) const
    { return thread_of(event); }
};

template <typename Map>
struct CustomEdgeInfo {
    Map& map;

    explicit CustomEdgeInfo(Map& map) : map(map) { }

    template <typename Key, typename Info>
    void operator()(Key const& key, Info const& l1, Info const& l2) const {
        typename Map::reference info = map[key];
        info.l1_info = l1;
        info.l2_info = l2;
    }
};

struct CustomVertexInfo {
    template <typename Key, typename Event>
    void operator()(Key const&, Event const&) const {
        // Nothing.
    }
};

/**
 * Algorithm building a lock graph from a range of events.
 *
 * Depending on the `SilentlyIgnoreOtherEvents` template parameter,
 * unexpected events trigger an exception or are ignored silently.
 *
 * In all cases, the algorithm provides the basic exception guarantee.
 */
template <bool SilentlyIgnoreOtherEvents, typename Iterator, typename Graph>
void build_lock_graph(Iterator first, Iterator last, Graph& graph) {
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

    // The first event must be a segment_hop, because generating a
    // segment_hop is the first thing we do when a thread is started.
    // The only case where the first event is not a segment_hop is
    // for the main thread, in which case it can be an acquire too.
    // We deduce the thread we're processing from the first event.
    DeduceThisThread<ThreadId> deduce;
    ThreadId this_thread = boost::apply_visitor(deduce, *first);

    typedef typename boost::property_map<
                Graph, boost::edge_bundle_t
            >::type EdgeBundleMap;

    typedef EventVisitor<
                Graph,
                CustomVertexInfo,
                CustomEdgeInfo<EdgeBundleMap>,
                SilentlyIgnoreOtherEvents
            > Visitor;

    CustomVertexInfo vertex_info;
    EdgeBundleMap edge_bundle = get(boost::edge_bundle, graph);
    CustomEdgeInfo<EdgeBundleMap> edge_info(edge_bundle);

    Visitor visitor(graph, this_thread, vertex_info, edge_info);
    for (; first != last; ++first)
        boost::apply_visitor(visitor, *first);
}

template <bool SilentlyIgnoreOtherEvents, typename Range, typename Graph>
void build_lock_graph(Range const& range, Graph& graph) {
    build_lock_graph<SilentlyIgnoreOtherEvents>(
        boost::begin(range), boost::end(range), graph);
}
} // end namespace build_lock_graph_detail

namespace core {
    using build_lock_graph_detail::build_lock_graph;
}
} // end namespace d2

#endif // !D2_CORE_BUILD_LOCK_GRAPH_HPP
