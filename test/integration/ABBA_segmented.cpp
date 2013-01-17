
#include "mock.hpp"


int main(int argc, char const* argv[]) {
    d2::mock::integration_test start(argc, argv, __FILE__);

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

    t0.start();
    t0.join();

    t1.start();
    t1.join();
}
