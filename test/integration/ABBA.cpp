/**
 * This test makes sure that we detect the basic deadlock between t0 and t1.
 * The deadlock happens if t0 holds A while t1 holds B. Both threads will be
 * waiting for the other to release its lock, hence creating a deadlock.
 */

#include <d2/mock.hpp>


int main(int argc, char const* argv[]) {
    d2::mock::mutex A, B;

    d2::mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2::mock::thread t1([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    t0.start();
    t1.start();

    t1.join();
    t0.join();

    d2::mock::integration_test::Streak streak{t0, A, B};
    d2::mock::integration_test::Deadlock dlock{
            {t0, A, B},
            {t0, A, B}
    };

    integration_test.verify_deadlocks(
            {
                {t0, A, B},
                {t1, B, A}
            }
    );
}
