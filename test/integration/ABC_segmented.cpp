
#include "mock.hpp"


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B, C;

    d2::mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2::mock::thread t1([&] {
        B.lock();
            C.lock();
            C.unlock();
        B.unlock();
    });

    d2::mock::thread t2([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t1.start();

    t0.join();
    t2.start();

    t1.join();
    t2.join();

    integration_test.verify_deadlocks(/* none */);
}
