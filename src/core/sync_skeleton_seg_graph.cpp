/**
 * This file implements the `build_segmentation_graph` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/build_segmentation_graph.hpp>
#include <d2/core/events.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/detail/decl.hpp>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <dyno/istream_iterator.hpp>
#include <ostream>


namespace d2 {
namespace synchronization_skeleton_detail {
D2_DECL void
synchronization_skeleton::build_segmentation_graph(Stream& stream) {
    typedef dyno::istream_iterator<
                Stream, core::events::non_thread_specific
            > Iterator;

    Iterator first(stream), last;
    static bool const ignore_other_events = true;
    core::build_segmentation_graph<ignore_other_events>(first, last, sg_);
}

namespace {
    template <typename SegmentMap>
    class SegmentWriter {
        SegmentMap segments_;

    public:
        explicit SegmentWriter(SegmentMap const& segments)
            : segments_(segments)
        { }

        template <typename VertexDescriptor>
        void operator()(std::ostream& os, VertexDescriptor const& v) const {
            os << "[segment=" << get(segments_, v) << ']';
        }
    };
} // end anonymous namespace

D2_DECL void
synchronization_skeleton::print_segmentation_graph(std::ostream& os) const {
    typedef boost::property_map<
                core::SegmentationGraph, boost::vertex_bundle_t
            >::const_type SegmentMap;

    boost::write_graphviz(os, sg_,
        SegmentWriter<SegmentMap>(boost::get(boost::vertex_bundle, sg_)));
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
