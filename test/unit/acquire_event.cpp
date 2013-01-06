/**
 * This file contains unit tests for the `AcquireEvent` event.
 */

#include <d2/acquire_event.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"


namespace {
    struct AcquireEventTest : ::testing::Test {
        std::stringstream stream;
        d2::Thread thread;
        d2::SyncObject lock;

        void SetUp() {
            lock = d2::SyncObject((unsigned)0xDEADBEEF);
            thread = d2::Thread((unsigned)0XBADC0FFEE);
            stream.unsetf(std::ios::skipws);
        }
    };
} // end anonymous namespace

TEST_F(AcquireEventTest, save_and_load_a_single_event_without_info) {
    d2::AcquireEvent saved(lock, thread);
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the acquire event");

    d2::AcquireEvent loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the acquire event");

    ASSERT_EQ(saved, loaded);
}

TEST_F(AcquireEventTest, save_and_load_a_single_event_with_info) {
    d2::AcquireEvent saved(lock, thread);
    saved.info.init_call_stack();
    stream << saved;
    EXPECT_TRUE(stream && "failed to save the acquire event");

    d2::AcquireEvent loaded;
    stream >> loaded;
    EXPECT_TRUE(stream && "failed to load the acquire event");

    ASSERT_EQ(saved, loaded);
}

TEST_F(AcquireEventTest, save_and_load_several_acquire_events_without_info) {
    using namespace boost::assign;
    std::vector<d2::AcquireEvent> saved;
    saved += d2::AcquireEvent(lock, thread),
             d2::AcquireEvent(lock, thread),
             d2::AcquireEvent(lock, thread);

    stream << karma::format(*karma::stream, saved);
    EXPECT_TRUE(stream && "failed to save the acquire events");

    std::vector<d2::AcquireEvent> loaded;
    stream >> qi::match(*qi::stream, loaded);
    EXPECT_TRUE(stream && "failed to load the acquire events");

    ASSERT_EQ(saved, loaded);
}

TEST_F(AcquireEventTest, save_and_load_several_acquire_events_with_info) {
    using namespace boost::assign;
    std::vector<d2::AcquireEvent> saved;
    saved += d2::AcquireEvent(lock, thread),
             d2::AcquireEvent(lock, thread),
             d2::AcquireEvent(lock, thread);
    BOOST_FOREACH(d2::AcquireEvent& event, saved)
        event.info.init_call_stack();

    stream << karma::format(*karma::stream, saved);
    EXPECT_TRUE(stream && "failed to save the acquire events");

    std::vector<d2::AcquireEvent> loaded;
    stream >> qi::match(*qi::stream, loaded);
    EXPECT_TRUE(stream && "failed to load the acquire events");

    ASSERT_EQ(saved, loaded);
}
