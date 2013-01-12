/**
 * This file contains unit tests for the `JoinEvent` event.
 */

#include <d2/events/join_event.hpp>
#include <d2/segment.hpp>
#include "test_base.hpp"


namespace {
    struct JoinEventTest : ::testing::Test {
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

TEST_F(JoinEventTest, save_and_load_a_single_join_event) {
    d2::JoinEvent saved(a, b, c);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the join event");

    d2::JoinEvent loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the join event");

    ASSERT_EQ(saved, loaded);
}

TEST_F(JoinEventTest, save_and_load_several_join_events) {
    using namespace boost::assign;
    std::vector<d2::JoinEvent> saved;
    saved += d2::JoinEvent(a, b, c),
             d2::JoinEvent(a, b, c),
             d2::JoinEvent(a, b, c);

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<d2::JoinEvent>(stream));
    EXPECT_TRUE(stream && "failed to save the join events");

    std::vector<d2::JoinEvent> loaded;
    std::copy(std::istream_iterator<d2::JoinEvent>(stream),
              std::istream_iterator<d2::JoinEvent>(),
              std::back_inserter(loaded));
    EXPECT_TRUE(stream.eof() && "failed to load the join events");

    ASSERT_EQ(saved, loaded);
}
