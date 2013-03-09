/**
 * Do not detect a deadlock between three threads and three locks if the
 * threads are synchronized in such a way that makes it impossible for them
 * to run at the same time.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B, C;

    d2mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            C.lock();
            C.unlock();
        B.unlock();
    });

    d2mock::thread t2([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t0.join();
        t2.start();

        t1.join();
        t2.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {/* none */});
}
