
#include "mock.hpp"


// This deadlock is missed if the "transitive closure" is not computed while
// building the graph because the B lock makes the A->B->C->A cycle have
// two edges (A->B)(B->C) within the same thread (t0). If the transitive
// closure of the `is held while locking' relation is computed during the
// building of the lock graph, the found cycle is A->C->A instead, which is
// valid.
int main(int argc, char const* argv[]) {
    mock::begin_integration_test(argc, argv, __FILE__);

    mock::mutex A, B, C;

    mock::thread t0([&] {
        A.lock();
            B.lock();
                C.lock();
                C.unlock();
            B.unlock();
        A.unlock();
    });

    mock::thread t1([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    mock::end_integration_test();
}
