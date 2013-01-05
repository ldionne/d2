/**
 * This file contains unit tests for the `FilesystemDispatcher` class.
 */

#include <d2/filesystem_dispatcher.hpp>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>


namespace fs = boost::filesystem;

namespace {
class FilesystemDispatcherTest : public ::testing::Test {
    void SetUp() {
        root = fs::unique_path();
        fs::create_directory(root);
    }

    void TearDown() {
        fs::remove_all(root);
    }

public:
    fs::path root;
};
} // end anonymous namespace


TEST_F(FilesystemDispatcherTest, no_test_yet) {

}
