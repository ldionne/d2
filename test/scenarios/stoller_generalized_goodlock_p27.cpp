/**
 * Scenario taken from a powerpoint named
 * `Detecting potential deadlocks with static analysis and run-time monitoring`
 * by Scott D. Stoller. Unfortunately, I can't find the original file.
 * The scenario is at page 27 of the powerpoint. A deadlock should be detected.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex L1, L2, L3, L4;

    d2mock::thread t0([&] {
        L1.lock();
            L2.lock();
            L2.unlock();
        L1.unlock();

        L1.lock();
            L3.lock();
                L4.lock();
                L4.unlock();
            L3.unlock();
        L1.unlock();
    });

    d2mock::thread t1([&] {
        L1.lock();
            L2.lock();
            L2.unlock();
        L1.unlock();

        L2.lock();
            L4.lock();
                L3.lock();
                L3.unlock();
            L4.unlock();
        L2.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds L1, L3 and waits for L4
                    {t0, L1, L3, L4},
                    // t1 holds L2, L4 and waits for L3
                    {t1, L2, L4, L3}
                }
            });
}
