/**
 * This file contains unit tests for the `StartEvent` event.
 */

#include <d2/events/start_event.hpp>
#include <d2/segment.hpp>
#include "test_base.hpp"


namespace {
    struct StartEventTest : ::testing::Test {
        std::stringstream stream;
        d2::Segment a, b, c;

        void SetUp() {
            stream.unsetf(std::ios::skipws);
            a += 100;
            b += 200;
            c += 300;
        }
    };
} // end anonymous namespace

TEST_F(StartEventTest, save_and_load_a_single_start_event) {
    d2::StartEvent saved(a, b, c);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the start event");

    d2::StartEvent loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the start event");

    ASSERT_EQ(saved, loaded);
}

TEST_F(StartEventTest, save_and_load_several_start_events) {
    using namespace boost::assign;
    std::vector<d2::StartEvent> saved;
    saved += d2::StartEvent(a, b, c),
             d2::StartEvent(a, b, c),
             d2::StartEvent(a, b, c);

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<d2::StartEvent>(stream));
    EXPECT_TRUE(stream && "failed to save the start events");

    std::vector<d2::StartEvent> loaded;
    std::copy(std::istream_iterator<d2::StartEvent>(stream),
              std::istream_iterator<d2::StartEvent>(),
              std::back_inserter(loaded));
    EXPECT_TRUE(stream.eof() && "failed to load the start events");

    ASSERT_EQ(saved, loaded);
}
