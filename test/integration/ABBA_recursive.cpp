
#include <d2/mock.hpp>


int main(int argc, char const* argv[]) {
    d2::mock::recursive_mutex A, B;

    d2::mock::thread t0([&] {
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

    d2::mock::thread t1([&] {
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

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    integration_test.verify_deadlocks({
        {
            {t0, A, B},
            {t1, B, A}
        }
    });
}
