
#include "mock.hpp"


int main(int argc, char const* argv[]) {
    mock::begin_integration_test(argc, argv, __FILE__);

    mock::mutex A, B, C;

    mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    mock::thread t1([&] {
        B.lock();
            C.lock();
            C.unlock();
        B.unlock();
    });

    mock::thread t2([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    t0.start();
    t1.start();

    t0.join();
    t2.start();

    t1.join();
    t2.join();

    mock::end_integration_test();
}
