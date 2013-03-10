/**
 * This file implements the `feed_lock_graph` method of the
 * `synchronization_skeleton` class.
 */

#define D2_SOURCE
#include <d2/core/build_lock_graph.hpp>
#include <d2/core/events.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <d2/detail/decl.hpp>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <dyno/istream_iterator.hpp>
#include <ostream>


namespace d2 {
namespace synchronization_skeleton_detail {
D2_DECL void synchronization_skeleton::feed_lock_graph(Stream& stream) {
    typedef dyno::istream_iterator<
                Stream, core::events::thread_specific
            > Iterator;

    Iterator first(stream), last;
    static bool const ignore_unrelated_events = true;
    core::build_lock_graph<ignore_unrelated_events>(first, last, lg_);
}

namespace {
    template <typename LockIdMap>
    class LockIdWriter {
        LockIdMap lock_ids_;

    public:
        explicit LockIdWriter(LockIdMap const& lock_ids)
            : lock_ids_(lock_ids)
        { }

        template <typename VertexDescriptor>
        void operator()(std::ostream& os, VertexDescriptor const& v) const {
            os << "[lock_id=" << get(lock_ids_, v) << ']';
        }
    };
} // end anonymous namespace

D2_DECL void
synchronization_skeleton::print_lock_graph(std::ostream& os) const {
    typedef boost::property_map<
                core::LockGraph, boost::vertex_bundle_t
            >::const_type LockIdMap;

    boost::write_graphviz(os, lg_,
        LockIdWriter<LockIdMap>(boost::get(boost::vertex_bundle, lg_)),
        boost::make_label_writer(boost::get(boost::edge_bundle, lg_)));
}
} // end namespace synchronization_skeleton_detail
} // end namespace d2
