/**
 * This test makes sure that the same locking pattern repeated in two
 * different functions will still trigger two different deadlock potentials.
 * For example, f() and g() perform the exact same thing. If we build the
 * graph/do the analysis naively, we could end up ignoring one of these two
 * deadlock potentials because one of them seems redundant. However, since
 * they happen in two different functions, it is pertinent to report both
 * deadlock potentials because it may be non obvious in real code.
 */

#include <d2/mock.hpp>


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B;

    auto f = [&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();

        // redundancy, but in a different function.
        [&] {
            A.lock();
                B.lock();
                B.unlock();
            A.unlock();
        }();
    };

    auto g = [&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    };

    d2::mock::thread t0(f), t1(g);

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t1.start();
    t1.join();
    t0.join();

    // We should detect the same deadlock twice. The information associated
    // to each deadlock should also be different, but we don't check that now.
    integration_test.verify_deadlocks({
        {
            {t0, B, A},
            {t1, A, B}
        }, {
            {t0, B, A},
            {t1, A, B}
        }
    });
}
