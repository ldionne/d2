/**
 * This is a modified version of the `ABBA_gatelock_in_parent_thread` scenario.
 * Here, the `G` gatelock is released by `t1` while `t3` is still running
 * free, so the deadlock between `t3` and `t2` could actually happen.
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
            t3.start();
        G.unlock(); // early release here
            t3.join();
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
                    // t3 holds L1 and waits for L2
                    {t3, L1, L2},
                    // t2 holds G, L2 and waits for L1
                    {t2, G, L2, L1}
                }
            });
}
