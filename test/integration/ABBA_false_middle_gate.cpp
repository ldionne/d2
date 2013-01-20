/**
 * This test makes sure that we do detect the deadlock potential between t0
 * and t1 even if there is a lock that could be misinterpreted as a gatelock.
 * Effectively, the G lock is not a gatelock because if t0 holds A and G while
 * t1 holds B (other scenarios are possible), then t0 and t1 are deadlocked.
 */

#include <d2/mock.hpp>


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B, G;

    d2::mock::thread t0([&] {
        A.lock();
            G.lock();
                B.lock();
                B.unlock();
            G.unlock();
        A.unlock();
    });

    d2::mock::thread t1([&] {
        B.lock();
            G.lock();
                A.lock();
                A.unlock();
            G.unlock();
        B.unlock();
    });

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    integration_test.verify_deadlocks(
        {
            {t0, A, G, B},
            {t1, B, G, A}
        }
    );
}
