/**
 * Scenario taken from:
 * `http://www.cs.sunysb.edu/~stoller/papers/deadlock-IBM-2010.pdf`
 * on page 3. A deadlock should be detected, and some false positives
 * should be avoided.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex G, L1, L2;

    d2mock::thread t3([&] {
        L1.lock();
            L2.lock();
            L2.unlock();
        L1.unlock();
    });

    d2mock::thread t1([&] {
        G.lock();
            L1.lock();
                L2.lock();
                L2.unlock();
            L1.unlock();
        G.unlock();

        t3.start();
        t3.join();

        L2.lock();
            L1.lock();
            L1.unlock();
        L2.unlock();
    });

    d2mock::thread t2([&] {
        G.lock();
            L2.lock();
                L1.lock();
                L1.unlock();
            L2.unlock();
        G.unlock();
    });

    auto test_main = [&] {
        t1.start();
        t2.start();

        t2.join();
        t1.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t2 holds G, L2, and waits for L1
                    {t2, G, L2, L1},
                    // t3 holds L1 and waits for L2
                    {t3, L1, L2}
                }
            });
}
