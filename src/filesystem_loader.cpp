/**
 * This file implements the `FilesystemLoader` class.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/events/acquire_event.hpp>
#include <d2/events/join_event.hpp>
#include <d2/events/release_event.hpp>
#include <d2/events/segment_hop_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/filesystem_loader.hpp>

#include <boost/assert.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <iostream>


namespace d2 {

FilesystemLoader::unspecified_event_range
FilesystemLoader::load_events(Ifstream& ifs) {
    using namespace boost::spirit::qi;
    ifs.unsetf(std::ios::skipws);
    unspecified_event_range events;

    stream_parser<char, AcquireEvent> acquire;
    stream_parser<char, ReleaseEvent> release;
    stream_parser<char, StartEvent> start;
    stream_parser<char, JoinEvent> join;
    stream_parser<char, SegmentHopEvent> hop;

    ifs >> match(
            *(lexeme[acquire] | release | start | join | hop)
        , events);

    BOOST_ASSERT_MSG(ifs, "unable to load the events. grammar problem?");
    return events;
}

} // end namespace d2
