
#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B, C;

    d2mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            C.lock();
            C.unlock();
        B.unlock();
    });

    d2mock::thread t2([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();
        t2.start();

        t2.join();
        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    {t0, A, B},
                    {t1, B, C},
                    {t2, C, A}
                }
            });
}
