/*!
 * @file
 * This file contains unit tests for the `d2::trackable_thread` class.
 */

#include <d2/trackable_thread.hpp>

#include <boost/assert.hpp>
namespace {

} // end anonymous namespace

template <typename T>
struct identity {
    typedef T result_type;
    result_type operator()(T x) const {
        return x;
    }
};

int main() {
    identity<int> id;
    d2::thread_function<identity<int> > tf(id);
    int i = 10;
    BOOST_ASSERT(10 == tf(i));
}
