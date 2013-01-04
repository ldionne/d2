/**
 * This file implements the `load_events` function from the logging API.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/detail/event_io.hpp>
#include <d2/events.hpp>

#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <istream>
#include <iterator>
#include <string>
#include <vector>


namespace d2 {

namespace {
    static detail::event_parser<std::string::const_iterator> parse_event;
}

D2_API extern std::vector<Event> load_events(std::istream& source) {
    source.unsetf(std::ios::skipws);
    std::string const input((std::istream_iterator<char>(source)),
                             std::istream_iterator<char>());

    std::vector<Event> events;
    std::string::const_iterator first(boost::begin(input)),
                                last(boost::end(input));
    bool success = boost::spirit::qi::parse(first, last,
                                            *(parse_event >> '\n'), events);
    (void)success;
    BOOST_ASSERT_MSG(success && first == last,
                            "unable to parse events using the qi grammar");

    return events;
}

} // end namespace d2
