
#include <d2/logging.hpp>
#include <iostream>


int main() {
    d2::set_event_sink(&std::cout);
    d2::enable_event_logging();

    unsigned int t0 = 70, t1 = 71, A = 4, B = 8;
    d2::notify_start(t0, t1);
        d2::notify_acquire(A, t1, __FILE__, __LINE__);
            d2::notify_acquire(B, t1, __FILE__, __LINE__);
            d2::notify_release(B, t1);
        d2::notify_release(A, t1);

        d2::notify_acquire(B, t0, __FILE__, __LINE__);
            d2::notify_acquire(A, t0, __FILE__, __LINE__);
            d2::notify_release(A, t0);
        d2::notify_release(B, t0);
    d2::notify_join(t0, t1);

    d2::disable_event_logging();
}
