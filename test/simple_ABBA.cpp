
#include "mock.hpp"
#include <d2/logging.hpp>
#include <ostream>


int main() {
    d2::set_event_sink(&std::cout);
    d2::enable_event_logging();

    mock_thread t0, t1;
    mock_mutex A, B;

    t0.start(t1);
        A.lock_in(t1);
            B.lock_in(t1);
            B.unlock_in(t1);
        A.unlock_in(t1);

        B.lock_in(t0);
            A.lock_in(t0);
            A.unlock_in(t0);
        B.unlock_in(t0);
    t0.join(t1);

    d2::disable_event_logging();
}
