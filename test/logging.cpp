/**
 * This file contains unit tests for the logging of events.
 */

#include <d2/logging.hpp>

#include <boost/assign.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>


using namespace d2;
using namespace d2::detail;
using namespace boost::assign;
using boost::begin;
using boost::end;

TEST(logging, log_simple_event) {
    std::stringstream repo;

    unsigned short t = 0, l = 1;
    set_event_sink(&repo);
    enable_event_logging();
    notify_release(l, t);
    disable_event_logging();

    std::vector<event> actual(load_events(repo));
    release_event expected((sync_object(l)), thread(t));
    ASSERT_TRUE(expected == boost::get<release_event>(actual[0]));
}
