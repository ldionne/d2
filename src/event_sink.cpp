/**
 * This file implements the d2/event_sink.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/config.hpp>
#include <d2/detail/event_io.hpp>
#include <d2/event_sink.hpp>

#include <boost/assert.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/spirit/include/karma.hpp>


namespace d2 {

EventSink::~EventSink() { }

namespace detail {
namespace {
    struct OstreamWrapperAsFunction {
        OstreamWrapper& wrapper_;

        explicit OstreamWrapperAsFunction(OstreamWrapper& wrapper)
            : wrapper_(wrapper)
        { }

        typedef void result_type;

        result_type operator()(char c) {
            wrapper_.put(c);
        }
    };

    typedef boost::function_output_iterator<OstreamWrapperAsFunction>
                                                   OstreamWrapperIterator;

    static event_generator<OstreamWrapperIterator> generate_event;
} // end anonymous namespace

D2_API extern void generate(OstreamWrapper& os, Event const& event) {
    bool const success = boost::spirit::karma::generate(
                    OstreamWrapperIterator(OstreamWrapperAsFunction(os)),
                    generate_event << '\n',
                    event);

    BOOST_ASSERT_MSG(success,
                    "unable to generate the event using the karma generator");
}
} // end namespace detail

} // end namespace d2
