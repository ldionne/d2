/**
 * This tests checks that we detect the same deadlock happening in two
 * different threads. This test is motivated by the fact that we could miss
 * the deadlock in one of the threads if the cycle finding algorithm did not
 * check for all the parallel edges between two vertices in a cycle.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B;

    d2mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2mock::thread t2([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();
        t2.start();

        t2.join();
        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds A, and waits for B
                    {t0, A, B},
                    // t2 holds B, and waits for A
                    {t2, B, A}
                }, {
                    // t1 holds A, and waits for B
                    {t1, A, B},
                    // t2 holds B, and waits for A
                    {t2, B, A}
                }
            });
}
