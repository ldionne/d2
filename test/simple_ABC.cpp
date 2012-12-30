
#include "mock.hpp"
#include <d2/logging.hpp>

#include <ostream>


int main() {
    d2::set_event_sink(&std::cout);
    d2::enable_event_logging();

    mock_mutex A, B, C;

    mock_thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    mock_thread t1([&] {
        B.lock();
            C.lock();
            C.unlock();
        B.unlock();
    });

    mock_thread t2([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    t0.start();
    t1.start();
    t2.start();

    t2.join();
    t1.join();
    t0.join();

    d2::disable_event_logging();
}