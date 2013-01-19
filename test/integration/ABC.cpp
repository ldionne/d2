
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
    t2.start();

    t2.join();
    t1.join();
    t0.join();

    integration_test.verify_deadlocks({
        {
            {t0, A, B},
            {t1, B, C},
            {t2, C, A}
        }
    });
}
