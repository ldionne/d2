/**
 * Do not detect a deadlock when a gatelock is present in the parent thread.
 *
 * More specifically, t3 can't deadlock with t2 because t1 needs to hold G
 * in order to start t3, and t2 needs to hold G in order to enter the
 * state in which it _could_ deadlock with t3.
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
        // Here, G prevents t3 from running in parallel with the body of t2,
        // so the deadlock is avoided.
        G.lock();
            t3.start();
            t3.join();
        G.unlock();
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

    return d2mock::check_scenario(test_main, argc, argv, {/* nothing */});
}
