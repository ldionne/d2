/**
 * This file defines the segmentation graph data structure used during the
 * post-mortem program analysis.
 */

#ifndef D2_SEGMENTATION_GRAPH_HPP
#define D2_SEGMENTATION_GRAPH_HPP

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
            if (!SilentlyIgnoreOtherEvents) {
                UnexpectedEventException exc("encountered unexpected event");
                exc.faulty_event = event;
                throw exc;
            }
        }

        void operator()(StartEvent const& event) {
            Segment parent_segment = event.parent;
            Segment child_segment = event.child;
            Segment new_parent_segment = event.new_parent;

            // Segments:      parent    n+1    n+2
            // Parent thread:   o________o
            // Child thread:     \______________o
            add_vertex(new_parent_segment, graph);
            add_vertex(child_segment, graph);
            add_edge(parent_segment, new_parent_segment, graph);
            add_edge(parent_segment, child_segment, graph);
        }

        void operator()(JoinEvent const& event) {
            Segment parent_segment = event.parent;
            Segment child_segment = event.child;
            Segment new_parent_segment = event.new_parent;

            // Note: Below, the (parent, child, n) segments are not
            //       necessarily ordered that way. `Any thread` can refer
            //       to any thread, including `parent` or `child`.
            // Segments:      parent    child    n    n+1
            // Parent thread:   o______________________o
            // Child thread:              o___________/
            // Any thread:                       o
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
        BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<SegmentationGraph>));
        BOOST_CONCEPT_ASSERT((boost::VertexMutablePropertyGraphConcept<
                                                        SegmentationGraph>));

        BOOST_CONCEPT_ASSERT((boost::InputIterator<Iterator>));

        typedef typename boost::vertex_property_type<SegmentationGraph>::type
                                                                    Segment;
        typedef typename boost::iterator_reference<Iterator>::type Event;

        if (first == last)
            return;

        // The first event should be a StartEvent. We deduce the initial
        // segment from it. If the first event is not a StartEvent, we can't
        // continue because we really need to know the initial segment.
        StartEvent const* initial_event = boost::get<StartEvent>(&*first);
        if (initial_event == NULL) {
            UnexpectedEventException exc("the first event is not a StartEvent");
            // This can throw std::bad_alloc because of hold_any.
            // If we are out of memory, we don't care about our bad first
            // event anyway, so we let it throw.
            exc.faulty_event = variant_to<AnyEvent>(*first);
            throw exc;
        }
        add_vertex(initial_event->parent, graph);

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
 * a segmentation graph. If either `u` or `v` are not associated to any
 * vertex in the segmentation graph, then `u` does not happen before `v`,
 * i.e. it is not an error.
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
    return u_ && v_ && boost::graph::is_reachable(*u_, *v_, graph);
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
