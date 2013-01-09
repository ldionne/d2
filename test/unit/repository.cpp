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
    repository[threads[0]].seekg(0); // rewind
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
        sources_sinks[threads[i]].seekg(0); // rewind
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

TEST_F(RepositoryTest, reload_previous_repository) {
    {
        Repository first(root);
        ASSERT_TRUE(first.empty());
        // Save a dummy value to create streams.
        for (unsigned int i = 0; i < threads.size(); ++i)
            first[threads[i]] << i;
        ASSERT_FALSE(first.empty());
    }
    {
        Repository second(root);
        ASSERT_FALSE(second.empty());
        for (unsigned int i = 0; i < threads.size(); ++i) {
            unsigned int saved;
            second[threads[i]] >> saved;
            ASSERT_EQ(i, saved);
        }
    }
}


struct UnaryCategory {
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, UnaryCategory const&) {
        return os << "UnaryCategory", os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, UnaryCategory&) {
        std::string name;
        is >> name;
        BOOST_ASSERT(name == "UnaryCategory");
        return is;
    }
};

struct HybridMappingPolicy {
    template <typename Category, typename Stream>
    struct apply
        : boost::mpl::apply<d2::boost_unordered_map, Category, Stream>
    { };

    template <typename Stream>
    struct apply<UnaryCategory, Stream>
        : boost::mpl::apply<d2::unary_map, UnaryCategory, Stream>
    { };
};

TEST_F(RepositoryTest, reload_previous_repository_multiple_categories) {
    typedef boost::mpl::vector<d2::Thread, UnaryCategory> Categories;
    typedef d2::Repository<Categories, HybridMappingPolicy> Repository;

    {
        Repository first(root);
        ASSERT_TRUE(first.empty());
        for (unsigned int i = 0; i < threads.size(); ++i)
            first[threads[i]] << i;

        UnaryCategory special_stream;
        first[special_stream] << (unsigned int)88888;

        ASSERT_FALSE(first.empty());
    }
    {
        Repository second(root);
        ASSERT_FALSE(second.empty());
        for (unsigned int i = 0; i < threads.size(); ++i) {
            unsigned int saved;
            second[threads[i]] >> saved;
            ASSERT_EQ(i, saved);
        }

        UnaryCategory special_stream;
        unsigned int special;
        second[special_stream] >> special;
        ASSERT_EQ(88888, special);
    }
}

TEST_F(RepositoryTest, throws_on_invalid_repo_path) {
    // Create a file and then try to create a repository at that path.
    std::ofstream ofs(root.c_str());
    ASSERT_THROW({
        Repository repository(root);
    }, d2::InvalidRepositoryPathException);
}
