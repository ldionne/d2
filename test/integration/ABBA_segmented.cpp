/**
 * This test makes sure that we do not find a deadlock when the threads
 * involved in the deadlock have a start/join relationship such that it is
 * impossible for them to run at the same time, i.e. a deadlock is not
 * possible.
 */

#include <d2/mock.hpp>


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B;

    d2::mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2::mock::thread t1([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t0.join();

    t1.start();
    t1.join();

    integration_test.verify_deadlocks({/* none */});
}
