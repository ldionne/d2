
#include "mock.hpp"


int main(int argc, char const* argv[]) {
    if (!mock::begin_integration_test(argc, argv, __FILE__))
        return EXIT_FAILURE;

    mock::recursive_mutex A, B;

    mock::thread t0([&] {
        A.lock();
        A.lock();
        A.lock();
            B.lock();
            B.lock();
            B.unlock();
            B.unlock();
        A.unlock();
        A.unlock();
        A.unlock();
    });

    mock::thread t1([&] {
        B.lock();
        B.lock();
        B.lock();
        B.lock();
            A.lock();
            A.lock();
            A.lock();
            A.unlock();
            A.unlock();
            A.unlock();
        B.unlock();
        B.unlock();
        B.unlock();
        B.unlock();
    });

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    mock::end_integration_test();
    return EXIT_SUCCESS;
}
