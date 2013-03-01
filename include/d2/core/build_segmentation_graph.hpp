/**
 * This file defines an algorithm to construct a directed graph describing
 * the relative order of thread starts and joins in a program.
 */

#ifndef D2_CORE_BUILD_SEGMENTATION_GRAPH_HPP
#define D2_CORE_BUILD_SEGMENTATION_GRAPH_HPP

#include <d2/events/exceptions.hpp>
#include <d2/events/join_event.hpp>
#include <d2/events/start_event.hpp>

#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/optional.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/static_visitor.hpp>
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
namespace build_segmentation_graph_detail {
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

    template <typename Graph, typename Segment>
    struct EventVisitor : boost::static_visitor<void> {
        Graph& graph;

        explicit EventVisitor(Graph& sg) : graph(sg) { }

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

    template <typename Iterator, typename Graph>
    result_type operator()(Iterator first, Iterator last, Graph& graph) const {
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
        BOOST_CONCEPT_ASSERT((boost::MutableGraphConcept<Graph>));
        BOOST_CONCEPT_ASSERT((boost::VertexMutablePropertyGraphConcept<Graph>));
#endif

        BOOST_CONCEPT_ASSERT((boost::InputIterator<Iterator>));

        typedef typename boost::vertex_property_type<Graph>::type Segment;

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

        EventVisitor<Graph, Segment> visitor(graph);
        do {
            boost::apply_visitor(visitor, *first);
        } while (++first != last);
    }

    template <typename Range, typename Graph>
    result_type operator()(Range const& range, Graph& graph) const {
        return (*this)(boost::begin(range), boost::end(range), graph);
    }
};

/**
 * Return whether the segment represented by `u` happens before the segment
 * represented by `v` according to `graph`.
 *
 * @note `graph` should have been built using `build_segmentation_graph` in
 *       order for this result to make any sense.
 */
template <typename Graph>
bool happens_before(typename boost::graph_traits<Graph>::vertex_descriptor u,
                    typename boost::graph_traits<Graph>::vertex_descriptor v,
                    Graph const& graph) {
    // They are not the same vertex and v is reachable from u.
    return u != v && boost::graph::is_reachable(u, v, graph);
}

template <typename Graph>
bool happens_before(typename boost::graph_traits<Graph>::vertex_descriptor u,
                    typename Graph::vertex_name_type const& v_,
                    Graph const& graph) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    boost::optional<Vertex> v = find_vertex(v_, graph);
    return v && happens_before(u, *v, graph);
}

template <typename Graph, typename VertexNameOrDescriptor>
bool happens_before(typename Graph::vertex_name_type const& u_,
                    VertexNameOrDescriptor const& v,
                    Graph const& graph) {
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    boost::optional<Vertex> u = find_vertex(u_, graph);
    return u && happens_before(*u, v, graph);
}
} // end namespace build_segmentation_graph_detail

namespace core {
    using build_segmentation_graph_detail::build_segmentation_graph;
    using build_segmentation_graph_detail::happens_before;
}
} // end namespace d2

#endif // !D2_CORE_BUILD_SEGMENTATION_GRAPH_HPP
