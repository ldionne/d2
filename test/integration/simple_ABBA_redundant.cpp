
#include "mock.hpp"

#include <cstddef>


static std::size_t const REPETITIONS = 100;

int main(int argc, char const* argv[]) {
    mock::begin_integration_test(argc, argv, __FILE__);

    mock::mutex A, B;

    mock::thread t0([&] {
        for (std::size_t i = 0; i < REPETITIONS; ++i) {
            A.lock();
                B.lock();
                B.unlock();
            A.unlock();
        }
    });

    mock::thread t1([&] {
        for (std::size_t i = 0; i < REPETITIONS; ++i) {
            B.lock();
                A.lock();
                A.unlock();
            B.unlock();
        }
    });

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    mock::end_integration_test();
}
