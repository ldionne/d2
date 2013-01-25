/**
 * This file defines the segmentation graph data structure used during the
 * post-mortem program analysis.
 */

#ifndef D2_SEGMENTATION_GRAPH_HPP
#define D2_SEGMENTATION_GRAPH_HPP

#include <d2/detail/variant_to.hpp>
#include <d2/events/exceptions.hpp>
#include <d2/events/join_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/segment.hpp>

#include <boost/assert.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/optional.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/variant.hpp>
#include <cstddef> // for NULL
#include <typeinfo>


namespace boost {
namespace graph {
    /**
     * Return whether vertex `v` is reachable from vertex `u`.
     * @note This is an extension to the existing `is_reachable`, which
     *       requires passing a property map.
     */
    template <typename Graph>
    bool is_reachable(typename graph_traits<Graph>::vertex_descriptor u,
                      typename graph_traits<Graph>::vertex_descriptor v,
                      Graph const& g) {
        one_bit_color_map<> map(num_vertices(g));
        return is_reachable(u, v, g, map);
    }
} // end namespace graph
} // end namespace boost

namespace d2 {

/**
 * Directed acyclic graph representing the order of starts and joins between
 * the threads of a program.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                                Segment> SegmentationGraph;

/**
 * Function object building a segmentation graph from a range of events.
 * The events in the range should be `StartEvent`s or `JoinEvent`s. Depending
 * on the `SilentlyIgnoreOtherEvents` template parameter, other events will
 * be ignored or an `UnexpectedEventException` will be thrown.
 *
 * Either way (`SilentlyIgnoreOtherEvents` or not), the functor provides
 * the basic exception guarantee.
 */
template <bool SilentlyIgnoreOtherEvents = true>
class build_segmentation_graph {

    template <typename SegmentationGraph, typename Segment>
    struct EventVisitor : boost::static_visitor<void> {
        SegmentationGraph& graph;

        explicit EventVisitor(SegmentationGraph& sg) : graph(sg) { }

        template <typename Event>
        void operator()(Event const& event) {
            if (!SilentlyIgnoreOtherEvents)
                D2_THROW(EventTypeException()
                            << ExpectedType("StartEvent or JoinEvent")
                            << ActualType(typeid(event).name()));
        }

        void operator()(StartEvent const& event) {
            Segment parent_segment = parent_of(event);
            Segment child_segment = child_of(event);
            Segment new_parent_segment = new_parent_of(event);

            // Segments:      parent     new_parent  child
            // Parent thread:   o____________o
            // Child thread:     \____________________o
            add_vertex(new_parent_segment, graph);
            add_vertex(child_segment, graph);
            add_edge(parent_segment, new_parent_segment, graph);
            add_edge(parent_segment, child_segment, graph);
        }

        void operator()(JoinEvent const& event) {
            Segment parent_segment = parent_of(event);
            Segment child_segment = child_of(event);
            Segment new_parent_segment = new_parent_of(event);

            // Segments:      parent    child       new_parent
            // Parent thread:   o______________________o
            // Child thread:              o___________/
            add_vertex(new_parent_segment, graph);
            add_edge(parent_segment, new_parent_segment, graph);
            add_edge(child_segment, new_parent_segment, graph);
        }
    };

public:
    typedef void result_type;

    template <typename Iterator, typename SegmentationGraph>
    result_type
    operator()(Iterator first, Iterator last, SegmentationGraph& graph) const {
        // We need to be able to add new vertices/edges to build the
        // segmentation graph.
        // Note: The VertexMutableGraph concept includes a check for
        //       remove_vertex(). However, since the SegmentationGraph is a
        //       named_graph using vecS as its out edge list, we can't use
        //       remove_vertex because of iterator invalidation as seen in
        //       https://svn.boost.org/trac/boost/ticket/7863. Therefore, we
        //       do not check the concept because we don't need remove_vertex
        //       anyway.
#if 0
        BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<SegmentationGraph>));
        BOOST_CONCEPT_ASSERT((boost::VertexMutablePropertyGraphConcept<
                                                        SegmentationGraph>));
#endif

        BOOST_CONCEPT_ASSERT((boost::InputIterator<Iterator>));

        typedef typename boost::vertex_property_type<SegmentationGraph>::type
                                                                    Segment;

        if (first == last)
            return;

        // The first event should be a StartEvent. We deduce the initial
        // segment from it. If the first event is not a StartEvent, we can't
        // continue because we really need to know the initial segment.
        typedef typename boost::iterator_reference<Iterator>::type Event;
        Event first_event = *first;
        StartEvent const* initial_event = boost::get<StartEvent>(&first_event);
        if (initial_event == NULL)
            D2_THROW(EventTypeException()
                        << ExpectedType("StartEvent")
                        << ActualType(typeid(first_event).name()));
        add_vertex(parent_of(*initial_event), graph);

        EventVisitor<SegmentationGraph, Segment> visitor(graph);
        do {
            boost::apply_visitor(visitor, *first);
        } while (++first != last);
    }

    template <typename Range, typename SegmentationGraph>
    result_type
    operator()(Range const& range, SegmentationGraph& graph) const {
        return (*this)(boost::begin(range), boost::end(range), graph);
    }
};

/**
 * Return whether segment `u` happens before segment `v` according to
 * a segmentation graph.
 *
 * If either `u` or `v` are not associated to any vertex in the segmentation
 * graph, then `u` does not happen before `v`, i.e. it is not an error.
 */
template <typename SegmentationGraph>
bool happens_before(
    typename boost::vertex_property_type<SegmentationGraph>::type const& u,
    typename boost::vertex_property_type<SegmentationGraph>::type const& v,
    SegmentationGraph const& graph) {
    typedef typename boost::graph_traits<SegmentationGraph>::vertex_descriptor
                                                            VertexDescriptor;
    boost::optional<VertexDescriptor> u_ = find_vertex(u, graph),
                                      v_ = find_vertex(v, graph);
    return (u_ && v_) &&    // They are in the graph
           (*u_ != *v_) &&  // They are not the same vertex
                            // There is a path from u to v
           boost::graph::is_reachable(*u_, *v_, graph);
}

} // end namespace d2

namespace boost {
namespace graph {

// This is to be able to refer to a vertex in the segmentation graph using the
// Segment associated to it.
template <> struct internal_vertex_name<d2::Segment> {
    typedef multi_index::identity<d2::Segment> type;
};

} // end namespace graph
} // end namespace boost

#endif // !D2_SEGMENTATION_GRAPH_HPP
