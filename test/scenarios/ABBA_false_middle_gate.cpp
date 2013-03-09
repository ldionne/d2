/**
 * This test makes sure that we detect the deadlock potential between two
 * threads even in presence of a lock that could be misinterpreted as a
 * gatelock.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B, G;

    d2mock::thread t0([&] {
        A.lock();
            G.lock();
                B.lock();
                B.unlock();
            G.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            G.lock();
                A.lock();
                A.unlock();
            G.unlock();
        B.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds A and G, and waits for B
                    {t0, A, G, B},
                    // t1 holds B, and waits for G
                    {t1, B, G}
                }, {
                    // t0 holds A, and waits for G
                    {t0, A, G},
                    // t1 holds B and G, and waits for A
                    {t1, B, G, A}
                }
            });
}
