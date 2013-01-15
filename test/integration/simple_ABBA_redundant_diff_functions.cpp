
#include "mock.hpp"


// This test makes sure that the same locking pattern repeated in two
// different functions will still trigger two different deadlock potentials.
// For example, f() and g() perform the exact same thing. If we build the
// graph/do the analysis naively, we could end up ignoring one of these two
// deadlock potentials because one of them seems redundant. However, since
// they happen in two different functions, it is pertinent to report both
// deadlock potentials because it may be non obvious in real code.
int main(int argc, char const* argv[]) {
    mock::mutex A, B;

    auto f = [&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    };

    auto g = [&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();

        f(); // redundancy, but in a different function.
    };

    auto h = [&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    };

    mock::thread t0(g), t1(h);

    if (!mock::begin_integration_test(argc, argv, __FILE__))
        return EXIT_FAILURE;

    t0.start();
    t1.start();
    t1.join();
    t0.join();

    mock::end_integration_test();
    return EXIT_SUCCESS;
}
