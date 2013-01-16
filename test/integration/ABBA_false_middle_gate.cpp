
#include "mock.hpp"


// This test makes sure that we do detect the deadlock potential between t0
// and t1 even if there is a lock that could be misinterpreted as a gatelock.
// Effectively, the G lock is not a gatelock because if t0 holds A and G while
// t1 holds B (other scenarios are possible), then t0 and t1 are deadlocked.
int main(int argc, char const* argv[]) {
    if (!mock::begin_integration_test(argc, argv, __FILE__))
        return EXIT_FAILURE;

    mock::mutex A, B, G;

    mock::thread t0([&] {
        A.lock();
            G.lock();
                B.lock();
                B.unlock();
            G.unlock();
        A.unlock();
    });

    mock::thread t1([&] {
        B.lock();
            G.lock();
                A.lock();
                A.unlock();
            G.unlock();
        B.unlock();
    });

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    mock::end_integration_test();
    return EXIT_SUCCESS;
}