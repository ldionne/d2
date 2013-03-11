/**
 * Detect the simplest acquisition order inconsistency between two locks and
 * two threads, when all locks are released in a different order than their
 * acquisition order.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B;

    d2mock::thread t0([&] {
        A.lock();
            B.lock();
            A.unlock(); // unordered
        B.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            A.lock();
            B.unlock(); // unordered too
        A.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds A, and waits for B
                    {t0, A, B},
                    // t1 holds B, and waits for A
                    {t1, B, A}
                }
            });
}
