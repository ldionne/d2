/**
 * This test makes sure that the same locking pattern repeated in two
 * different functions will still trigger two different deadlock potentials.
 * For example, f() and g() perform the exact same thing. If we build the
 * graph/do the analysis naively, we could end up ignoring one of these two
 * deadlock potentials because one of them seems redundant. However, since
 * they happen in two different functions, it is pertinent to report both
 * deadlock potentials because it may be non obvious in real code.
 */

#include "mock.hpp"


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B;

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

    d2::mock::thread t0(g), t1(h);

    d2::mock::integration_test start(argc, argv, __FILE__);

    t0.start();
    t1.start();
    t1.join();
    t0.join();
}
