/**
 * This file implements the event logging.
 */

#include <d2/detail/basic_mutex.hpp>
#include <d2/detail/event_io.hpp>
#include <d2/events.hpp>

#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>


namespace d2 {
namespace detail {

static basic_mutex sink_lock;
static bool is_enabled = false;
static std::ostream* event_sink = NULL;
static event_generator<std::ostream_iterator<char> > generate_event;

extern void push_event_impl(event const& e) {
    sink_lock.lock();
    if (is_enabled) {
        BOOST_ASSERT_MSG(event_sink != NULL,
                                    "logging events in an invalid NULL sink");

        bool success = boost::spirit::karma::generate(
                        std::ostream_iterator<char>(*event_sink),
                        generate_event << '\n',
                        e);
        (void)success;

        BOOST_ASSERT_MSG(success,
                    "unable to generate the event using the karma generator");
    }
    sink_lock.unlock();
}

} // end namespace detail

extern void set_event_sink(std::ostream* sink) {
    BOOST_ASSERT_MSG(sink != NULL, "setting an invalid NULL sink");
    detail::sink_lock.lock();
    detail::event_sink = sink;
    detail::sink_lock.unlock();
}

extern void disable_event_logging() {
    detail::sink_lock.lock();
    detail::is_enabled = false;
    detail::sink_lock.unlock();
}

extern void enable_event_logging() {
    detail::sink_lock.lock();
    detail::is_enabled = true;
    detail::sink_lock.unlock();
}

extern std::vector<event> load_events(std::istream& source) {
    detail::event_parser<std::string::const_iterator> parse_event;
    source.unsetf(std::ios::skipws);
    std::string const input((std::istream_iterator<char>(source)),
                             std::istream_iterator<char>());

    std::vector<event> events;
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
