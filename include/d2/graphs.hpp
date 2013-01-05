/**
 * This file defines the `LockGraph` and the `SegmentationGraph` types
 * used during the analysis.
 */

#ifndef D2_GRAPHS_HPP
#define D2_GRAPHS_HPP

#include <d2/detail/lock_debug_info.hpp>
#include <d2/segment.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/unordered_set.hpp>


namespace d2 {

/**
 * Directed acyclic graph representing the order of starts and joins between
 * the threads of a program.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                                Segment> SegmentationGraph;
typedef boost::graph_traits<SegmentationGraph>::vertex_descriptor Segment;

/**
 * Label present on the edges of a lock graph.
 */
struct LockGraphLabel : boost::equality_comparable<LockGraphLabel> {
    friend bool operator==(LockGraphLabel const& a, LockGraphLabel const& b) {
        // Note: We test the easiest first, i.e. the threads and segments,
        //       which are susceptible of being similar to integers.
        return a.s1 == b.s1 &&
               a.t == b.t &&
               a.s2 == b.s2 &&
               a.l1_info == b.l1_info &&
               a.l2_info == b.l2_info &&
               a.g == b.g;
    }

    inline LockGraphLabel() { }

    inline LockGraphLabel(detail::LockDebugInfo const& l1_info,
                          Segment s1,
                          Thread t,
                          boost::unordered_set<SyncObject> const& g,
                          Segment s2,
                          detail::LockDebugInfo const& l2_info)
        : l1_info(l1_info), s1(s1), t(t), g(g), s2(s2), l2_info(l2_info)
    { }

    detail::LockDebugInfo l1_info;
    Segment s1;
    Thread t;
    boost::unordered_set<SyncObject> g;
    Segment s2;
    detail::LockDebugInfo l2_info;
};

/**
 * Directed graph representing the contexts in which synchronization objects
 * were acquired by threads.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                        SyncObject, LockGraphLabel> LockGraph;

/**
 * Concept specification of a lock graph.
 */
template <typename G>
struct LockGraphConcept : boost::GraphConcept<G> {
    BOOST_MPL_ASSERT((boost::is_same<
                        typename boost::edge_property_type<G>::type,
                        LockGraphLabel
                    >));

    BOOST_MPL_ASSERT((boost::is_same<
                        typename boost::vertex_property_type<G>::type,
                        SyncObject
                    >));
};

} // end namespace d2

namespace boost {
namespace graph {
    // This is to be able to refer to a vertex in the lock graph using the
    // SyncObject associated to it.
    template <> struct internal_vertex_name<d2::SyncObject> {
        typedef multi_index::identity<d2::SyncObject> type;
    };

    // Idem for the SegmentationGraph with Segments.
    template <> struct internal_vertex_name<d2::Segment> {
        typedef multi_index::identity<d2::Segment> type;
    };

    // This is to satisfy the EdgeIndexGraph concept, which is
    // BOOST_CONCEPT_ASSERTed in tiernan_all_cycles even though
    // it is not required.
    void renumber_vertex_indices(d2::LockGraph const&);
} // end namespace graph
} // end namespace boost

#endif // !D2_GRAPHS_HPP
