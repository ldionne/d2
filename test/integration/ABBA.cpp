
#include "mock.hpp"


// This test makes sure that we detect the basic deadlock between t0 and t1.
// The deadlock happens if t0 holds A while t1 holds B. Both threads will be
// waiting for the other to release its lock, hence creating a deadlock.
int main(int argc, char const* argv[]) {
    if (!mock::begin_integration_test(argc, argv, __FILE__))
        return EXIT_FAILURE;

    mock::mutex A, B;

    mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    mock::thread t1([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    mock::end_integration_test();
    return EXIT_SUCCESS;
}
