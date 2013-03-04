/**
 * This file contains unit tests for the `filesystem` class.
 */

#include <d2/core/filesystem.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <gtest/gtest.h>
#include <iostream>


namespace bfs = boost::filesystem;

// Force instantiation of the class template.
template class d2::core::filesystem<bfs::fstream>;
template class d2::core::filesystem<bfs::ofstream>;
template class d2::core::filesystem<bfs::ifstream>;

namespace {
struct filesystem_test : testing::Test {
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

typedef d2::core::filesystem<bfs::fstream> Filesystem;

TEST_F(filesystem_test, create_filesystem) {
    Filesystem fs(root, std::ios::in | std::ios::out);
}
} // end anonymous namespace
