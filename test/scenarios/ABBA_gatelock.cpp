/**
 * This test makes sure that we do not detect a deadlock between two threads
 * and two locks if there is a gatelock blocking the cycle.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B, G;

    d2mock::thread t0([&] {
        G.lock();
            A.lock();
                B.lock();
                B.unlock();
            A.unlock();
        G.unlock();
    });

    d2mock::thread t1([&] {
        G.lock();
            B.lock();
                A.lock();
                A.unlock();
            B.unlock();
        G.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {/* none */});
}
