/**
 * This test makes sure that we present the locks held by a thread when
 * it deadlocks in their order of acquisition.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    d2mock::mutex A, B;
    d2mock::mutex aa, ab, ac, ad;

    d2mock::thread t0([&] {
        // We lock and unlock from ad to aa because they are created in
        // reverse order. Because of the mechanics involved with unique
        // lock identifiers, it is more likely that the sequence of held
        // locks we expect does not match the natural ordering of the
        // identifiers of the locks. Basically, we have less chances of
        // passing the test by accident if we lock from ad to aa.
        A.lock();
            ad.lock(); ac.lock(); ab.lock(); aa.lock();
                B.lock();
                B.unlock();
            aa.unlock(); ab.unlock(); ac.unlock(); ad.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    auto test_main = [&] {
        t0.start();
        t1.start();

        t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    {t0, A, ad, ac, ab, aa, B},
                    {t1, B, A}
                }
            });
}
