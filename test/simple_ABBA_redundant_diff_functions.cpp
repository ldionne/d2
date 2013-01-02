
#include "mock.hpp"
#include <d2/logging.hpp>

#include <ostream>


int main() {
    mock::mutex A, B;

    auto f = [&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    };

    auto g = [&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();

        f(); // redundancy, but in a different function.
    };

    auto h = [&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    };

    mock::thread t0(g), t1(h);

    d2::set_event_sink(&std::cout);
    d2::enable_event_logging();

    t0.start();
    t1.start();
    t1.join();
    t0.join();

    d2::disable_event_logging();
}
