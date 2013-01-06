/**
 * This file contains unit tests for the `FilesystemDispatcher` class.
 */

#include <d2/filesystem_dispatcher.hpp>
#include "test_base.hpp"


namespace {
class FilesystemDispatcherTest : public ::testing::Test {
    void SetUp() {
        root = fs::temp_directory_path();
        root /= fs::unique_path();
        std::cout << "test directory is: " << root << std::endl;
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
