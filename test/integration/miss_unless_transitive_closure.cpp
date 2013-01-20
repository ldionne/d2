/**
 * This deadlock is missed if the transitive closure of the
 * `X is held by this thread while locking Y' relation is not computed when
 * building the lock graph because the B lock causes the (A, B)(B, C)(C, A)
 * cycle to have two edges (A, B)(B, C) within the same thread (t0). If the
 * transitive closure is computed, the cycle we find is (A, C)(C, A) instead,
 * which is valid. Thus, the transitive closure allows us to "jump" over the
 * B lock because there is a DIRECT edge from A to C.
 */

#include <d2/mock.hpp>


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B, C;

    d2::mock::thread t0([&] {
        A.lock();
            B.lock();
                C.lock();
                C.unlock();
            B.unlock();
        A.unlock();
    });

    d2::mock::thread t1([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    integration_test.verify_deadlocks(
        {
            {t0, A, B, C},
            {t1, C, A}
        }
    );
}
