/**
 * This file contains unit tests for the `synchronization_skeleton` class.
 */

#include <d2/core/synchronization_skeleton.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <gtest/gtest.h>
#include <iostream>


namespace bfs = boost::filesystem;

// Force instantiation of the class template.
template class d2::core::synchronization_skeleton<
                d2::core::filesystem<bfs::fstream> >;
template class d2::core::synchronization_skeleton<
                d2::core::filesystem<bfs::ofstream> >;
template class d2::core::synchronization_skeleton<
                d2::core::filesystem<bfs::ifstream> >;

namespace {
struct synchronization_skeleton_test : testing::Test {
    bfs::path test_dir, root;

    void SetUp() {
        test_dir = bfs::temp_directory_path() / bfs::unique_path();
        root = test_dir / bfs::unique_path();
        bfs::create_directory(test_dir);
    }

    void TearDown() {
        if (HasFailure())
            std::clog << "test directory at: " << test_dir << '\n';
        else
            bfs::remove_all(test_dir);
    }
};

typedef d2::core::synchronization_skeleton<
            d2::core::filesystem<bfs::fstream>
        > SyncSkeleton;

TEST_F(synchronization_skeleton_test, create_skeleton) {
    SyncSkeleton skel(root, std::ios::in | std::ios::out);
}
} // end anonymous namespace
