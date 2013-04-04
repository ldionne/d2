/**
 * This test checks whether we detect a deadlock arising from a parent thread
 * trying to join a child who requires a lock held by its parent in order to
 * finish.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex G;

    d2mock::thread t1([&] {
        G.lock();
        G.unlock();
    });

    d2mock::thread t0([&] {
        t1.start();
        // if t0 acquires G before t1 does, t1 can never be joined
        G.lock();
        t1.join();
        G.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t0.join();
    };

    // Right now, we have no way of encoding these kinds of deadlocks,
    // and we're obviously not detecting them too. For now, we'll always
    // make the test fail.
#if 1
    (void)test_main;
    return EXIT_FAILURE;
#else
    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds G, and waits for t1 to join
                    {t0, G, join(t1)},
                    // t1 waits for G, and will never be joined
                    {t1, G}
                }
            });
#endif
}
