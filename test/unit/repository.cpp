/**
 * This file contains unit tests for the `Repository` class.
 */

#include <d2/events/acquire_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/repository.hpp>
#include <d2/segment.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>
#include "test_base.hpp"


namespace {
    typedef boost::mpl::vector<d2::Thread> TestPolicy;
    typedef d2::Repository<TestPolicy> Repository;

    struct RepositoryTest : ::testing::Test {
        std::vector<d2::Thread> threads;
        std::vector<d2::SyncObject> locks;
        std::vector<d2::Segment> segments;
        fs::path root;

        void SetUp() {
            for (unsigned int i = 0; i < 100; ++i) {
                threads.push_back(d2::Thread(i));
                locks.push_back(d2::SyncObject(i));
                segments.push_back(d2::Segment() + i);
            }

            root = fs::temp_directory_path();
            root /= fs::unique_path();
            std::cout << "test directory is: " << root << std::endl;
        }

        void TearDown() {
            fs::remove_all(root);
        }
    };
} // end anonymous namespace

TEST_F(RepositoryTest, should_be_empty_at_the_beginning) {
    Repository repository(root);
    EXPECT_TRUE(repository.empty());
}

TEST_F(RepositoryTest, save_and_load_into_one_thread) {
    Repository repository(root);
    d2::AcquireEvent saved(locks[10], threads[0]);
    repository[threads[0]] << saved;
    EXPECT_FALSE(repository.empty());

    d2::AcquireEvent loaded;
    repository[threads[0]] >> loaded;
    ASSERT_EQ(saved, loaded);
}

TEST_F(RepositoryTest, get_all_keys) {
    Repository repository(root);

    // Save a dummy value to open the handles.
    for (unsigned int i = 0; i < threads.size(); ++i)
        repository[threads[i]] << i;

    Repository::key_view<d2::Thread>::type
        sources_sinks = repository.keys<d2::Thread>();

    // We must do an unordered comparison.
    boost::unordered_set<d2::Thread>
                        expected(threads.begin(), threads.end()),
                        actual(sources_sinks.begin(), sources_sinks.end());
    ASSERT_TRUE(expected == actual);
}

TEST_F(RepositoryTest, map_threads_to_sources_and_sinks) {
    Repository repository(root);

    // Save a dummy value to open the handles.
    for (unsigned int i = 0; i < threads.size(); ++i)
        repository[threads[i]] << i;

    Repository::item_view<d2::Thread>::type
        sources_sinks = repository.items<d2::Thread>();

    for (unsigned int i = 0; i < threads.size(); ++i) {
        unsigned int loaded;
        sources_sinks[threads[i]] >> loaded;
        ASSERT_EQ(i, loaded);
    }
}

TEST_F(RepositoryTest, get_all_streams_only) {
    Repository repository(root);

    // Save a dummy value to open the handles.
    for (unsigned int i = 0; i < threads.size(); ++i)
        repository[threads[i]] << i;

    Repository::value_view<d2::Thread>::type
        sources_sinks = repository.values<d2::Thread>();
    ASSERT_EQ(sources_sinks.size(), threads.size());
}
