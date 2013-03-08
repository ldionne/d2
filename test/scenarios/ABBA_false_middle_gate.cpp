/**
 * This test makes sure that we do detect the deadlock potential between t0
 * and t1 even if there is a lock that could be misinterpreted as a gatelock.
 * Effectively, the G lock is not a gatelock because if t0 holds A and G while
 * t1 holds B (other scenarios are possible), then t0 and t1 are deadlocked.
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
                    {t0, A, G, B},
                    {t1, B, G, A}
                }
            });
}
