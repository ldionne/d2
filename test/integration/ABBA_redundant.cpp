/**
 * This test makes sure that we do not report multiple deadlocks when there
 * are redundant locking patterns. For example, we do not wish to report
 * more than 1 potential deadlock between t0 and t1, even though the
 * pattern leading to a deadlock is encountered 100 times in each thread.
 */

#include <d2mock.hpp>

#include <cstddef>


int main(int argc, char const* argv[]) {
    static std::size_t const REPETITIONS = 100;

    d2mock::mutex A, B;

    d2mock::thread t0([&] {
        for (std::size_t i = 0; i < REPETITIONS; ++i) {
            A.lock();
                B.lock();
                B.unlock();
            A.unlock();
        }
    });

    d2mock::thread t1([&] {
        for (std::size_t i = 0; i < REPETITIONS; ++i) {
            B.lock();
                A.lock();
                A.unlock();
            B.unlock();
        }
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
