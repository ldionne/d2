/**
 * This file implements the `build_segmentation_graph` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/build_segmentation_graph.hpp>
#include <d2/core/events.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/core/thread_id.hpp>
#include <d2/detail/decl.hpp>

#include <boost/assert.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/variant/static_visitor.hpp>
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

struct GatelockFiller : boost::static_visitor<void> {
    explicit GatelockFiller(InitialGatelocksMap& initial_gatelocks)
        : initial_gatelocks_(initial_gatelocks)
    { }

    void operator()(core::events::start const& event) const {
        BOOST_ASSERT_MSG(initial_gatelocks_.count(thread_of(event)) == 0,
            "redundantly adding initial gatelocks to the same thread");
        initial_gatelocks_.insert(thread_of(event), );
    }

    void operator()(core::events::join const& event) const {

    }

private:
    InitialGatelocksMap& initial_gatelocks_;
};

D2_DECL void
synchronization_skeleton::fill_initial_gatelocks(Stream& stream,
                                    InitialGatelocksMap& initial_gatelocks) {
    typedef dyno::istream_iterator<
                Stream, core::events::non_thread_specific
            > Iterator;

    GatelockFiller gatelock_filler(initial_gatelocks);
    for (Iterator first(stream), last; first != last; ++first)
        boost::apply_visitor(gatelock_filler, *first);
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
