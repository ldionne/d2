
#include "mock.hpp"
#include <d2/logging.hpp>

#include <cstddef>
#include <ostream>


static std::size_t const REPETITIONS = 100;

int main() {
    d2::set_event_sink(&std::cout);
    d2::enable_event_logging();

    mock_mutex A, B;

    mock_thread t0([&] {
        for (std::size_t i = 0; i < REPETITIONS; ++i) {
            A.lock();
                B.lock();
                B.unlock();
            A.unlock();
        }
    });

    mock_thread t1([&] {
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

    d2::disable_event_logging();
}
