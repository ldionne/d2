/**
 * Detect the simplest acquisition order inconsistency involving two threads
 * and two recursive locks. The locks are acquired several times recursively.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::recursive_mutex A, B;

    d2mock::thread t0([&] {
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

    d2mock::thread t1([&] {
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

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds A, and waits for B
                    {t0, A, B},
                    // t1 holds B, and waits for A
                    {t1, B, A}
                }
            });
}
