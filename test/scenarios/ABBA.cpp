/**
 * This test makes sure that we detect the basic deadlock between t0 and t1.
 * The deadlock happens if t0 holds A while t1 holds B. Both threads will be
 * waiting for the other to release its lock, hence creating a deadlock.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B;

    d2mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    {t0, A, B},
                    {t1, B, A}
                }
            });
}
