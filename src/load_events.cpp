/**
 * This file implements the `load_events` function from the logging API.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/events.hpp>

#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_match.hpp>
#include <istream>
#include <vector>


namespace d2 {

D2_API extern std::vector<Event> load_events(std::istream& source) {
    using namespace boost::spirit::qi;

    stream_parser<char, AcquireEvent> acquire;
    stream_parser<char, ReleaseEvent> release;
    stream_parser<char, StartEvent> start;
    stream_parser<char, JoinEvent> join;

    std::vector<Event> events;
    source >> match(
            *((lexeme[acquire] | release | start | join) >> '\n')
        , events);

    BOOST_ASSERT_MSG(source, "unable to parse events using the qi grammar");
    return events;
}

} // end namespace d2
