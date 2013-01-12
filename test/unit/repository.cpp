/**
 * This file contains unit tests for the `Repository` class.
 */

#include <d2/events/acquire_event.hpp>
#include <d2/events/start_event.hpp>
#include <d2/repository.hpp>
#include <d2/segment.hpp>
#include <d2/sync_object.hpp>
#include <d2/thread.hpp>

#include <boost/assert.hpp>
#include <boost/filesystem.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/unordered_set.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>


namespace fs = boost::filesystem;
namespace mpl = boost::mpl;

namespace d2 {
namespace test {

struct RepositoryTest : ::testing::Test {
    std::vector<Thread> threads;
    std::vector<SyncObject> locks;
    std::vector<Segment> segments;
    fs::path root;

    struct UnaryKey {
        friend std::ostream& operator<<(std::ostream& os, UnaryKey const&) {
            return os << "UnaryKey", os;
        }

        friend std::istream& operator>>(std::istream& is, UnaryKey&) {
            std::string name;
            is >> name;
            BOOST_ASSERT(name == "UnaryKey");
            return is;
        }
    };

    struct MixedMappingPolicy {
        template <typename Category, typename Stream>
        struct apply
            : mpl::apply<boost_unordered_map, Category, Stream>
        { };

        template <typename Stream>
        struct apply<UnaryKey, Stream>
            : mpl::apply<unary_map, UnaryKey, Stream>
        { };
    };

    typedef mpl::vector<Thread> ThreadKeys;
    typedef mpl::vector<Thread, UnaryKey> MixedKeys;

    typedef Repository<ThreadKeys> ThreadRepository;
    typedef Repository<MixedKeys, MixedMappingPolicy> MixedRepository;

    void SetUp() {
        for (unsigned int i = 0; i < 100; ++i) {
            threads.push_back(Thread(i));
            locks.push_back(SyncObject(i));
            segments.push_back(Segment() + i);
        }

        root = fs::temp_directory_path();
        root /= fs::unique_path();
        std::cout << "test directory is: " << root << std::endl;
    }

    void TearDown() {
        fs::remove_all(root);
    }
};

TEST_F(RepositoryTest, should_be_empty_at_the_beginning) {
    ThreadRepository repository(root);
    EXPECT_TRUE(repository.empty());
}

TEST_F(RepositoryTest, save_and_load_into_one_thread) {
    ThreadRepository repository(root);
    AcquireEvent saved(locks[10], threads[0]);
    repository[threads[0]] << saved;
    EXPECT_FALSE(repository.empty());

    AcquireEvent loaded;
    repository[threads[0]].seekg(0); // rewind
    repository[threads[0]] >> loaded;
    ASSERT_EQ(saved, loaded);
}

TEST_F(RepositoryTest, get_all_keys) {
    ThreadRepository repository(root);

    // Save a dummy value to open the handles.
    for (unsigned int i = 0; i < threads.size(); ++i)
        repository[threads[i]] << i;

    ThreadRepository::key_view<Thread>::type
        repo_threads = repository.keys<Thread>();

    // We must do an unordered comparison.
    boost::unordered_set<Thread>
                        expected(threads.begin(), threads.end()),
                        actual(repo_threads.begin(), repo_threads.end());
    ASSERT_TRUE(expected == actual);
}

TEST_F(RepositoryTest, get_all_streams_only) {
    ThreadRepository repository(root);

    // Save a dummy value to open the handles.
    for (unsigned int i = 0; i < threads.size(); ++i)
        repository[threads[i]] << i;

    ThreadRepository::value_view<Thread>::type
        sources_sinks = repository.values<Thread>();
    ASSERT_EQ(sources_sinks.size(), threads.size());
}

// Compile time test.
TEST_F(RepositoryTest, get_non_const_reference_on_stream_from_view) {
    if (false) {
        ThreadRepository repository(root);
        ThreadRepository::value_view<Thread>::type
            sources_sinks = repository.values<Thread>();

        // Try to acquire a non-const reference to one of the streams
        std::istream& is = *sources_sinks.begin();
        (void)is;
    }
}

TEST_F(RepositoryTest, reload_previous_repository) {
    {
        ThreadRepository first(root);
        ASSERT_TRUE(first.empty());
        // Save a dummy value to create streams.
        for (unsigned int i = 0; i < threads.size(); ++i)
            first[threads[i]] << i;
        ASSERT_FALSE(first.empty());
    }
    {
        ThreadRepository second(root);
        ASSERT_FALSE(second.empty());
        for (unsigned int i = 0; i < threads.size(); ++i) {
            unsigned int saved;
            second[threads[i]] >> saved;
            ASSERT_EQ(i, saved);
        }
    }
}

TEST_F(RepositoryTest, reload_previous_repository_multiple_categories) {
    {
        MixedRepository first(root);
        ASSERT_TRUE(first.empty());
        for (unsigned int i = 0; i < threads.size(); ++i)
            first[threads[i]] << i;

        UnaryKey special_stream;
        first[special_stream] << (unsigned int)88888;

        ASSERT_FALSE(first.empty());
    }
    {
        MixedRepository second(root);
        ASSERT_FALSE(second.empty());
        for (unsigned int i = 0; i < threads.size(); ++i) {
            unsigned int saved;
            second[threads[i]] >> saved;
            ASSERT_EQ(i, saved);
        }

        UnaryKey special_stream;
        unsigned int special;
        second[special_stream] >> special;
        ASSERT_EQ(88888, special);
    }
}

TEST_F(RepositoryTest, throws_on_invalid_repo_path) {
    // Create a file and then try to create a repository at that path.
    std::ofstream ofs(root.c_str());
    ASSERT_THROW({
        ThreadRepository repository(root);
    }, InvalidRepositoryPathException);
}

TEST_F(RepositoryTest, use_read_and_write_to_manipulate_streams) {
    ThreadRepository repository(root);
    ASSERT_TRUE(repository.empty());

    for (unsigned int i = 0; i < threads.size(); ++i)
        repository.write(threads[i], i);
    ASSERT_FALSE(repository.empty());

    for (unsigned int i = 0; i < threads.size(); ++i) {
        unsigned int loaded;
        repository[threads[i]].seekg(0); // rewind
        repository.read(threads[i], loaded);
        ASSERT_EQ(i, loaded);
    }
}

} // end namespace test
} // end namespace d2
