/**
 * This file contains unit tests for the `ReleaseEvent` event.
 */

#include <d2/events/release_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"


namespace {
    struct ReleaseEventTest : ::testing::Test {
        std::stringstream stream;
        d2::Thread thread;
        d2::SyncObject lock;

        void SetUp() {
            stream.unsetf(std::ios::skipws);
        }
    };
} // end anonymous namespace

TEST_F(ReleaseEventTest, save_and_load_a_single_release_event) {
    d2::ReleaseEvent saved(lock, thread);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the release event");

    d2::ReleaseEvent loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the release event");

    ASSERT_EQ(saved, loaded);
}

TEST_F(ReleaseEventTest, save_and_load_several_release_events) {
    using namespace boost::assign;
    std::vector<d2::ReleaseEvent> saved;
    saved += d2::ReleaseEvent(lock, thread),
             d2::ReleaseEvent(lock, thread),
             d2::ReleaseEvent(lock, thread);

    std::copy(saved.begin(), saved.end(),
        std::ostream_iterator<d2::ReleaseEvent>(stream));
    EXPECT_TRUE(stream && "failed to save the release events");

    std::vector<d2::ReleaseEvent> loaded;
    std::copy(std::istream_iterator<d2::ReleaseEvent>(stream),
              std::istream_iterator<d2::ReleaseEvent>(),
              std::back_inserter(loaded));
    EXPECT_TRUE(stream.eof() && "failed to load the release events");

    ASSERT_EQ(saved, loaded);
}
